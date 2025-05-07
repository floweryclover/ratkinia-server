//
// Created by floweryclover on 2025-04-11.
//

#include "Session.h"
#include "Errors.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

Session::Session(const SOCKET socket, const size_t sessionId)
    : Socket{ socket },
      SessionId{ sessionId },
      BufferCapacity{ RatkiniaProtocol::MessageMaxSize * 128 },
      receiveTempBuffer_{ std::make_unique<char[]>(RatkiniaProtocol::MessageMaxSize) },
      receiveBuffer_{ std::make_unique<char[]>(BufferCapacity) },
      receiveBufferBegin_{ 0 },
      receiveBufferEnd_{ 0 },
      receiveBufferSize_{ 0 },
      sendBuffer_{ std::make_unique<char[]>(BufferCapacity) },
      sendBufferBegin_{ 0 },
      sendBufferEnd_{ 0 }
{
    ZeroMemory(&IOContext_Receive, sizeof(IOContext_Receive));
    ZeroMemory(&IOContext_Send, sizeof(IOContext_Send));
}

Session::~Session()
{
    shutdown(Socket, SD_BOTH);
    closesocket(Socket);
}

bool Session::PostAccept(const HANDLE iocpHandle)
{
    if (nullptr
        == CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket),
                                  iocpHandle,
                                  reinterpret_cast<ULONG_PTR>(this),
                                  0))
    {
        ERR_PRINT_VARARGS("생성한 ClientSocket의 IOCP 핸들로의 연결 작업에 실패했습니다:", GetLastError());
        return false;
    }

    SOCKADDR_IN* localAddr;
    SOCKADDR_IN* remoteAddr;
    int localAddrLen;
    int remoteAddrLen;

    GetAcceptExSockaddrs(receiveBuffer_.get(),
                         0,
                         sizeof(SOCKADDR_IN) + 16,
                         sizeof(SOCKADDR_IN) + 16,
                         reinterpret_cast<sockaddr**>(&localAddr),
                         &localAddrLen,
                         reinterpret_cast<sockaddr**>(&remoteAddr),
                         &remoteAddrLen);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &remoteAddr->sin_addr, buf, INET_ADDRSTRLEN) == nullptr)
    {
        MessagePrinter::WriteLine("알 수 없는 클라이언트 접속");
    }
    else
    {
        MessagePrinter::WriteLine(buf, "접속");
    }

    return true;
}

bool Session::AcceptAsync(const SOCKET listenSocket, LPOVERLAPPED acceptOverlapped)
{
    if (FALSE
        == AcceptEx(listenSocket,
                    Socket,
                    receiveBuffer_.get(),
                    0,
                    sizeof(SOCKADDR_IN) + 16,
                    sizeof(SOCKADDR_IN) + 16,
                    nullptr,
                    acceptOverlapped))
    {
        const auto lastError = WSAGetLastError();
        if (lastError != ERROR_IO_PENDING)
        {
            ERR_PRINT_VARARGS("AcceptEx() 호출 실패:", lastError);
            return false;
        }
    }

    return true;
}

bool Session::ReceiveAsync()
{
    WSABUF wsabuf{};
    wsabuf.buf = receiveBuffer_.get() + receiveBufferBegin_;
    wsabuf.len = BufferCapacity - receiveBufferBegin_;

    IOContext_Receive.Type = IOType::Receive;

    DWORD dwFlags = 0;
    if (0
        != WSARecv(Socket,
                   &wsabuf,
                   1,
                   nullptr,
                   &dwFlags,
                   reinterpret_cast<LPOVERLAPPED>(&IOContext_Receive),
                   nullptr))
    {
        const auto errorCode = WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            ERR_PRINT_VARARGS("WSARecv() 호출 실패:", errorCode);
            return false;
        }
    }

    return true;
}

void Session::PostReceive(size_t bytesTransferred)
{
    receiveBufferSize_ += bytesTransferred;
    receiveBufferEnd_ = (receiveBufferEnd_ + bytesTransferred) % BufferCapacity;
}

