//
// Created by floweryclover on 2025-04-11.
//

#include "Session.h"
#include "Errors.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

Session::Session(const SOCKET socket, const size_t sessionId)
    : SessionId{ sessionId },
      BufferCapacity{ RatkiniaProtocol::MessageMaxSize * 128 },
      socket_{ socket },
      receiveTempBuffer_{ std::make_unique<char[]>(RatkiniaProtocol::MessageMaxSize) },
      receiveBuffer_{ std::make_unique<char[]>(BufferCapacity) },
      receiveBufferHead_{ 0 },
      receiveBufferTail_{ 0 },
      sendBuffer_{ std::make_unique<char[]>(BufferCapacity) },
      sendBufferHead_{ 0 },
      sendBufferTail_{ 0 }
{
    ZeroMemory(&IOContext_Receive, sizeof(IOContext_Receive));
    ZeroMemory(&IOContext_Send, sizeof(IOContext_Send));
}

Session::~Session()
{
    if (socket_ != INVALID_SOCKET)
    {
        Close();
    }
}

bool Session::TryPostAccept(HANDLE iocpHandle)
{
    if (nullptr
        == CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket_),
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

    addressRaw_ = *remoteAddr;
    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &remoteAddr->sin_addr, buf, INET_ADDRSTRLEN))
    {
        address_ = buf;
    }
    else
    {
        address_ = "알 수 없음";
    }
    MessagePrinter::WriteLine("[접속]", address_);

    return true;
}

bool Session::TryAcceptAsync(SOCKET listenSocket, LPOVERLAPPED acceptOverlapped)
{
    if (FALSE
        == AcceptEx(listenSocket,
                    socket_,
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

bool Session::TryReceiveAsync()
{
    const auto loadedReceiveBufferHead{ receiveBufferHead_->load(std::memory_order_acquire) };
    const auto loadedReceiveBufferTail{ receiveBufferTail_->load(std::memory_order_acquire) };
    WSABUF wsabuf{};
    wsabuf.buf = receiveBuffer_.get() + loadedReceiveBufferHead;
    wsabuf.len = loadedReceiveBufferTail >= loadedReceiveBufferHead ? BufferCapacity - loadedReceiveBufferTail : loadedReceiveBufferHead - loadedReceiveBufferTail - 1;

    IOContext_Receive.Type = IOType::Receive;

    if (wsabuf.len == 0)
    {
        return false;
    }

    if (receiveFlag_->exchange(true, std::memory_order_acq_rel))
    {
        return true;
    }

    DWORD dwFlags = 0;
    if (0
        != WSARecv(socket_,
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
            receiveFlag_->store(false, std::memory_order_release);
            ERR_PRINT_VARARGS("WSARecv() 호출 실패:", errorCode);
            return false;
        }
    }

    return true;
}

bool Session::TrySendAsync()
{
    const auto loadedSendBufferHead{ sendBufferHead_->load(std::memory_order_acquire) };
    const auto loadedSendBufferTail{ sendBufferTail_->load(std::memory_order_acquire) };
    WSABUF wsabuf{};
    wsabuf.buf = sendBuffer_.get() + loadedSendBufferHead;
    wsabuf.len = loadedSendBufferTail >= loadedSendBufferHead ? loadedSendBufferTail - loadedSendBufferHead : BufferCapacity - loadedSendBufferHead;

    if (wsabuf.len == 0
        || sendFlag_->exchange(true, std::memory_order_acq_rel))
    {
        return true;
    }

    IOContext_Send.Type = IOType::Send;

    if (0
        != WSASend(socket_,
                   &wsabuf,
                   1,
                   nullptr,
                   0,
                   reinterpret_cast<LPOVERLAPPED>(&IOContext_Send),
                   nullptr))
    {
        const auto errorCode = WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            sendFlag_->store(false, std::memory_order_release);
            ERR_PRINT_VARARGS("WSASend() 호출 실패:", errorCode);
            return false;
        }
    }

    return true;
}

void Session::Close()
{
    if (socket_ != INVALID_SOCKET)
    {
        shutdown(socket_, SD_BOTH);
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
        MessagePrinter::WriteLine("[접속 해제]", address_);
    }
}

bool Session::IsErasable()
{
    return !sendFlag_->load(std::memory_order_acquire) && !receiveFlag_->load(std::memory_order_acquire);
}

