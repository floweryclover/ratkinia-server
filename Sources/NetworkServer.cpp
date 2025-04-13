//
// Created by floweryclover on 2025-03-31.
//

#include "NetworkServer.h"
#include "MainServer.h"
#include "MessagePrinter.h"
#include "Errors.h"
#include <MSWSock.h>

using namespace RatkiniaServer;

NetworkServer::NetworkServer(MainServer& mainServer)
    : mainServer_{ mainServer },
      listenSocket_{ INVALID_SOCKET },
      iocpHandle_{ nullptr },
      newSessionId_{ 0 }
{
    acceptContexts_ = std::make_unique<AcceptContext[]>(AcceptPoolSize);
}

NetworkServer::~NetworkServer()
{
    shouldStop_.store(true, std::memory_order_release);
    shouldStop_.notify_all();

    for (auto& workerThread : workerThreads_)
    {
        workerThread.join();
    }

    if (listenSocket_ != INVALID_SOCKET)
    {
        closesocket(listenSocket_);
    }

    if (iocpHandle_ != nullptr)
    {
        CloseHandle(iocpHandle_);
    }
}

void NetworkServer::Start(const std::string& listenAddress, unsigned short listenPort)
{
    if (iocpHandle_ != nullptr)
    {
        ERR_PRINT_VARARGS("NetworkServer::Start()가 두 번 이상 호출되었습니다.");
        mainServer_.RequestTerminate();
        return;
    }

    iocpHandle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (iocpHandle_ == nullptr)
    {
        ERR_PRINT_VARARGS("IOCP 핸들 생성에 실패하였습니다.");
        mainServer_.RequestTerminate();
        return;
    }

    const auto threadCount = std::thread::hardware_concurrency();
    for (int i = 0; i < threadCount; ++i)
    {
        workerThreads_.emplace_back(&NetworkServer::WorkerThreadBody, this, i);
    }

    SOCKADDR_IN sockaddrIn{};
    sockaddrIn.sin_port = htons(listenPort);
    sockaddrIn.sin_family = AF_INET;
    if (inet_pton(AF_INET, listenAddress.c_str(), &sockaddrIn.sin_addr) != 1)
    {
        ERR_PRINT_VARARGS("IP 문자열 변환에 실패하였습니다:", listenAddress);
        mainServer_.RequestTerminate();
        return;
    }

    listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket_ == INVALID_SOCKET)
    {
        ERR_PRINT_VARARGS("리슨 소켓 생성에 실패하였습니다: ", WSAGetLastError());
        mainServer_.RequestTerminate();
        return;
    }

    if (nullptr == CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket_),
                                          iocpHandle_,
                                          reinterpret_cast<ULONG_PTR>(this),
                                          0))
    {
        ERR_PRINT_VARARGS("리슨 소켓을 IOCP 핸들에 연결시키는 작업에 실패하였습니다: ", GetLastError());
        mainServer_.RequestTerminate();
    }

    int option = 1;
    if (SOCKET_ERROR == setsockopt(listenSocket_,
                                   SOL_SOCKET,
                                   SO_REUSEADDR,
                                   reinterpret_cast<char*>(&option),
                                   sizeof(option)))
    {
        ERR_PRINT_VARARGS("리슨 소켓 옵션 설정에 실패하였습니다:", WSAGetLastError());
        mainServer_.RequestTerminate();
        return;
    }

    if (bind(listenSocket_, reinterpret_cast<sockaddr*>(&sockaddrIn), sizeof(SOCKADDR_IN)))
    {
        ERR_PRINT_VARARGS("리슨 소켓 바인딩에 실패하였습니다:", WSAGetLastError());
        mainServer_.RequestTerminate();
        return;
    }

    if (listen(listenSocket_, AcceptPoolSize))
    {
        ERR_PRINT_VARARGS("리슨 소켓 listen()에 실패하였습니다:", WSAGetLastError());
        mainServer_.RequestTerminate();
        return;
    }

    for (int i = 0; i < AcceptPoolSize; ++i)
    {
        if (!AcceptAsync(acceptContexts_[i]))
        {
            mainServer_.RequestTerminate();
            return;
        }
    }
}

void NetworkServer::WorkerThreadBody(const int threadId)
{
    MessagePrinter::WriteLine("IOCP 워커 스레드", threadId, "시작");
    while (!shouldStop_.load(std::memory_order_relaxed))
    {
        DWORD bytesTransferred = 0;
        LPOVERLAPPED overlapped = nullptr;
        ULONG_PTR completionKey = 0;
        if (GetQueuedCompletionStatus(iocpHandle_,
                                      &bytesTransferred,
                                      &completionKey,
                                      &overlapped,
                                      INFINITE) == FALSE)
        {
            MessagePrinter::WriteErrorLine("Error", GetLastError());
            if (completionKey != 0) // 타임아웃 외 오류
            {
                const OverlappedEx& context = *reinterpret_cast<OverlappedEx*>(overlapped);
                if (context.Type == IOType::Accept)
                {
                    auto& acceptContext = *reinterpret_cast<AcceptContext*>(reinterpret_cast<size_t>(overlapped)
                                                                            - 8);

                    ERR_PRINT_VARARGS("GQCS() 에러:",
                                      WSAGetLastError(),
                                      "Session Id:",
                                      acceptContext.SessionId);
                    sessions_.erase(acceptContext.SessionId);
                    if (!AcceptAsync(acceptContext))
                    {
                        mainServer_.RequestTerminate();
                        return;
                    }
                    continue;
                }
                MessagePrinter::WriteLine("IOCP 워커 스레드", threadId, "GQCS() 에러:", GetLastError());
            }
            continue;
        }

        if (completionKey == 0)
        {
            // 소켓 외 기타 작업들..
            continue;
        }

        const OverlappedEx& context = *reinterpret_cast<OverlappedEx*>(overlapped);


        if (context.Type == IOType::Accept)
        {
            MessagePrinter::WriteLine("Accept");
        }
        else if (context.Type == IOType::Receive)
        {
            MessagePrinter::WriteLine("Receive");
        }
        else
        {
            MessagePrinter::WriteLine("Send");
        }

        if (context.Type == IOType::Accept)
        {
            auto& acceptContext = *reinterpret_cast<AcceptContext*>(reinterpret_cast<size_t>(overlapped)
                                                                    - 8);
            Session& session = sessions_.at(acceptContext.SessionId);

            SOCKADDR_IN* localAddr;
            int localAddrlen;
            SOCKADDR_IN* remoteAddr;
            int remoteAddrlen;

            GetAcceptExSockaddrs(session.GetFilledReceiveBuffer().Pointer,
                                 0,
                                 sizeof(SOCKADDR_IN) + 16,
                                 sizeof(SOCKADDR_IN) + 16,
                                 reinterpret_cast<sockaddr**>(&localAddr),
                                 &localAddrlen,
                                 reinterpret_cast<sockaddr**>(&remoteAddr),
                                 &remoteAddrlen);

            char buf[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &remoteAddr->sin_addr, buf, INET_ADDRSTRLEN) == nullptr)
            {
                MessagePrinter::WriteLine("알 수 없는 클라이언트 접속");
            }
            else
            {
                MessagePrinter::WriteLine(buf, "접속");
            }
            if (!AcceptAsync(acceptContext))
            {
                mainServer_.RequestTerminate();
            }
            continue;
        }
    }
}

bool NetworkServer::AcceptAsync(AcceptContext& acceptContext)
{
    while (sessions_.contains(newSessionId_))
    {
        newSessionId_ += 1;
    }
    const auto sessionId = newSessionId_;

    const auto acceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (acceptSocket == INVALID_SOCKET)
    {
        ERR_PRINT_VARARGS("Accept 소켓 생성에 실패하였습니다:", WSAGetLastError());
        return false;
    }

    auto [iter, result] = sessions_.emplace(newSessionId_,
                                            Session{ acceptSocket, SessionBufferSize });
    auto& session = iter->second;

    const auto freeBufferData = session.GetFreeReceiveBuffer();
    if (freeBufferData.Size < sizeof(SOCKADDR_IN) * 2 + 32)
    {
        ERR_PRINT_VARARGS("세션 버퍼 크기가 충분하지 않습니다.");
        closesocket(acceptSocket);
        sessions_.erase(sessionId);
        return false;
    }

    ZeroMemory(&acceptContext, sizeof(AcceptContext));
    acceptContext.SessionId = sessionId;
    acceptContext.Context.Type = IOType::Accept;

    if (FALSE
        == AcceptEx(listenSocket_,
                    acceptSocket,
                    freeBufferData.Pointer,
                    0,
                    sizeof(SOCKADDR_IN) + 16,
                    sizeof(SOCKADDR_IN) + 16,
                    nullptr,
                    reinterpret_cast<LPOVERLAPPED>(&acceptContext.Context)))
    {
        const auto lastError = WSAGetLastError();
        if (lastError != ERROR_IO_PENDING)
        {
            ERR_PRINT_VARARGS("AcceptEx() 호출 실패:", lastError);
            closesocket(acceptSocket);
            sessions_.erase(sessionId);
            return false;
        }
    }

    return true;
}

