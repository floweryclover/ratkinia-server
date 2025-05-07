//
// Created by floweryclover on 2025-03-31.
//

#include "NetworkServer.h"
#include "MainServer.h"
#include "GameServer.h"
#include "MessagePrinter.h"
#include "GameServerTerminal.h"
#include "NetworkServerTerminal.h"
#include "Errors.h"
#include "google/protobuf/message_lite.h"
#include <MSWSock.h>

NetworkServer::NetworkServer(MainServer& mainServer,
                             NetworkServerTerminal& networkServerTerminal,
                             GameServerTerminal& gameServerTerminal)
    : mainServer_{ mainServer },
      networkServerTerminal_{ networkServerTerminal },
      gameServerTerminal_{ gameServerTerminal },
      listenSocket_{ INVALID_SOCKET },
      iocpHandle_{ nullptr },
      newSessionId_{ 0 },
      acceptContexts_{ std::make_unique<AcceptContext[]>(AcceptPoolSize) }
{

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
        ERR_PRINT_VARARGS("리슨 소켓 SO_REUSEADDR 설정에 실패하였습니다:", WSAGetLastError());
        mainServer_.RequestTerminate();
        return;
    }
    if (SOCKET_ERROR == setsockopt(listenSocket_,
                                   IPPROTO_TCP,
                                   TCP_NODELAY,
                                   reinterpret_cast<char*>(&option),
                                   sizeof(option)))
    {
        ERR_PRINT_VARARGS("리슨 소켓 TCP_NODELAY 설정에 실패하였습니다:", WSAGetLastError());
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
        acceptContexts_[i].SessionId = NullSessionId;
        if (!AcceptAsync())
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
                MessagePrinter::WriteLine("IOCP 워커 스레드", threadId, "GQCS() 에러:", GetLastError());
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
                    acceptContext.SessionId = NullSessionId;
                    if (!AcceptAsync())
                    {
                        mainServer_.RequestTerminate();
                        return;
                    }
                    continue;
                }
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
            auto& acceptContext = *reinterpret_cast<AcceptContext*>(reinterpret_cast<size_t>(overlapped));
            PostAccept(sessions_.at(acceptContext.SessionId));
            acceptContext.SessionId = NullSessionId;
        }
        else if (context.Type == IOType::Receive)
        {
            PostReceive(*reinterpret_cast<Session*>(completionKey), bytesTransferred);
        }
        else
        {
            MessagePrinter::WriteLine("Send");
        }
    }
}

void NetworkServer::TerminalObserverThreadBody()
{
    while (!shou)
}

bool NetworkServer::AcceptAsync()
{
    while (sessions_.contains(newSessionId_) || newSessionId_ == NullSessionId)
    {
        newSessionId_ += 1;
    }
    const auto sessionId = newSessionId_;

    AcceptContext* acceptContext = nullptr;
    for (int i = 0; i < AcceptPoolSize; ++i)
    {
        if (acceptContexts_[i].SessionId == NullSessionId)
        {
            acceptContext = &acceptContexts_[i];
            break;
        }
    }
    if (!acceptContext)
    {
        ERR_PRINT_VARARGS("AcceptContext 확보에 실패하였습니다.");
        return false;
    }

    const auto clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        ERR_PRINT_VARARGS("Accept 소켓 생성에 실패하였습니다:", WSAGetLastError());
        return false;
    }


    ZeroMemory(acceptContext, sizeof(AcceptContext));
    acceptContext->Context.Type = IOType::Accept;
    acceptContext->SessionId = sessionId;

    auto [iter, result] = sessions_.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(newSessionId_),
                                            std::forward_as_tuple(clientSocket, newSessionId_));
    auto& session = iter->second;

    if (!session.AcceptAsync(listenSocket_, reinterpret_cast<LPOVERLAPPED>(acceptContext)))
    {
        acceptContext->SessionId = NullSessionId;
        sessions_.erase(sessionId);
        return false;
    }

    return true;
}

void NetworkServer::PostAccept(Session& session)
{
    if (!session.PostAccept(iocpHandle_))
    {
        sessions_.erase(session.SessionId);
        return;
    }
    if (!session.ReceiveAsync())
    {
        sessions_.erase(session.SessionId);
    }
}

void NetworkServer::PostReceive(Session& session, const size_t bytesTransferred)
{
    session.PostReceive(bytesTransferred);
    while (session.TryPopMessage([this](auto sessionId, auto messageType, auto bodySize, auto body)
                                 {
                                     return gameServer_.PushMessage(sessionId,
                                                                    messageType,
                                                                    bodySize,
                                                                    body);
                                 },
                                 [this](auto sessionId)
                                 {
                                     OnMessagePushFailed(sessionId);
                                 }));
    if (!session.ReceiveAsync())
    {
        sessions_.erase(session.SessionId);
    }
}

void NetworkServer::OnMessagePushFailed(size_t sessionId)
{
    MessagePrinter::WriteErrorLine("NetworkServer -> GameServer 메시지 큐 Push에 실패하였습니다:", sessionId);
    sessions_.erase(sessionId);
}

void NetworkServer::DisconnectSession(uint64_t session)
{

}


