//
// Created by floweryclover on 2025-03-31.
//

#include "NetworkServer.h"
#include "MainServer.h"
#include "MessagePrinter.h"
#include "Errors.h"

NetworkServer::NetworkServer(NetworkServerChannel::SpscReceiver networkServerReceiver,
                             MainServerChannel::MpscSender mainServerSender,
                             GameServerChannel::MpscSender gameServerSender)
    : networkServerReceiver_{ std::move(networkServerReceiver) },
      mainServerSender_{ std::move(mainServerSender) },
      gameServerSender_{ std::move(gameServerSender) },
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
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }

    iocpHandle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (iocpHandle_ == nullptr)
    {
        ERR_PRINT_VARARGS("IOCP 핸들 생성에 실패하였습니다.");
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
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
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }

    listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket_ == INVALID_SOCKET)
    {
        ERR_PRINT_VARARGS("리슨 소켓 생성에 실패하였습니다: ", WSAGetLastError());
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }

    if (nullptr == CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket_),
                                          iocpHandle_,
                                          reinterpret_cast<ULONG_PTR>(this),
                                          0))
    {
        ERR_PRINT_VARARGS("리슨 소켓을 IOCP 핸들에 연결시키는 작업에 실패하였습니다: ", GetLastError());
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
    }

    int option = 1;
    if (SOCKET_ERROR == setsockopt(listenSocket_,
                                   SOL_SOCKET,
                                   SO_REUSEADDR,
                                   reinterpret_cast<char*>(&option),
                                   sizeof(option)))
    {
        ERR_PRINT_VARARGS("리슨 소켓 SO_REUSEADDR 설정에 실패하였습니다:", WSAGetLastError());
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }
    if (SOCKET_ERROR == setsockopt(listenSocket_,
                                   IPPROTO_TCP,
                                   TCP_NODELAY,
                                   reinterpret_cast<char*>(&option),
                                   sizeof(option)))
    {
        ERR_PRINT_VARARGS("리슨 소켓 TCP_NODELAY 설정에 실패하였습니다:", WSAGetLastError());
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }

    if (bind(listenSocket_, reinterpret_cast<sockaddr*>(&sockaddrIn), sizeof(SOCKADDR_IN)))
    {
        ERR_PRINT_VARARGS("리슨 소켓 바인딩에 실패하였습니다:", WSAGetLastError());
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }

    if (listen(listenSocket_, AcceptPoolSize))
    {
        ERR_PRINT_VARARGS("리슨 소켓 listen()에 실패하였습니다:", WSAGetLastError());
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }

    for (int i = 0; i < AcceptPoolSize; ++i)
    {
        acceptContexts_[i].SessionId = NullSessionId;
        AcceptAsync();
    }

    channelReceiverThread_ = std::thread{ &NetworkServer::ChannelReceiverThreadBody, this };
}

void NetworkServer::WorkerThreadBody(const int threadId)
{
    MessagePrinter::WriteLine("IOCP 워커 스레드", threadId, "시작");
    while (!shouldStop_.load(std::memory_order_acquire))
    {
        DWORD bytesTransferred = 0;
        LPOVERLAPPED overlapped = nullptr;
        ULONG_PTR completionKey = 0;
        if (FALSE == GetQueuedCompletionStatus(iocpHandle_,
                                               &bytesTransferred,
                                               &completionKey,
                                               &overlapped,
                                               1000))
        {
            if (completionKey != 0) // 타임아웃 외 오류
            {
                const OverlappedEx& context = *reinterpret_cast<OverlappedEx*>(overlapped);
                if (context.Type == IOType::Accept)
                {
                    auto& acceptContext = *reinterpret_cast<AcceptContext*>(overlapped);

                    ERR_PRINT_VARARGS("Accept 에러:",
                                      WSAGetLastError(),
                                      "Session Id:",
                                      acceptContext.SessionId);
                    DisconnectSession(acceptContext.SessionId);
                    acceptContext.SessionId = NullSessionId;
                    AcceptAsync();
                }
                else // 세션별 송수신 에러
                {
                    Session& session{ *reinterpret_cast<Session*>(completionKey) };
                    if (context.Type == IOType::Receive)
                    {
                        if (GetLastError() != ERROR_NETNAME_DELETED)
                        {
                            ERR_PRINT_VARARGS("세션", session.SessionId, "수신 에러:", GetLastError());
                        }
                        PostReceive(session, 0);
                    }
                    else
                    {
                        if (GetLastError() != ERROR_NETNAME_DELETED)
                        {
                            ERR_PRINT_VARARGS("세션", session.SessionId, "송신 에러:", GetLastError());
                        }
                        PostSend(session, 0);
                    }
                    DisconnectSession(session.SessionId);
                }
            }
            continue;
        }

        if (completionKey == 0)
        {
            // 소켓 외 기타 작업들..
            ERR_PRINT("CompletionKey가 NULL이었습니다.");
            continue;
        }

        const OverlappedEx& context = *reinterpret_cast<OverlappedEx*>(overlapped);

        if (context.Type == IOType::Accept)
        {
            {
                std::lock_guard lock{ sessionsMutex_ };

                auto& acceptContext = *reinterpret_cast<AcceptContext*>(reinterpret_cast<size_t>(overlapped));
                const auto sessionId = acceptContext.SessionId;
                acceptContext.SessionId = NullSessionId;

                PostAccept(sessions_.at(sessionId));
            }
            AcceptAsync();
        }
        else if (context.Type == IOType::Receive)
        {
            PostReceive(*reinterpret_cast<Session*>(completionKey), bytesTransferred);
        }
        else
        {
            PostSend(*reinterpret_cast<Session*>(completionKey), bytesTransferred);
        }
    }
    MessagePrinter::WriteLine("IOCP 워커 스레드", threadId, "종료");
}

void NetworkServer::ChannelReceiverThreadBody()
{
    MessagePrinter::WriteLine("NetworkServerChannel Receiver 스레드 시작");

    while (!shouldStop_.load(std::memory_order_acquire))
    {
        if (networkServerReceiver_.IsClosed())
        {
            break;
        }

        networkServerReceiver_.Wait();

        std::lock_guard lock{ sessionsMutex_ };
        while (const auto message = networkServerReceiver_.TryPeek())
        {
            const auto context{ message->Context };
            const auto messageType{ message->MessageType };
            const auto bodySize{ message->BodySize };
            if (!sessions_.contains(context))
            {
                networkServerReceiver_.Pop(*message);
                continue;
            }
            Session& session{ sessions_.at(context) };
            RatkiniaProtocol::MessageHeader header{};
            header.MessageType = htons(messageType);
            header.BodyLength = htons(bodySize);
            if (!session.TryEnqueueSendBuffer(reinterpret_cast<char*>(&header), sizeof(header))
                || !session.TryEnqueueSendBuffer(message->Body, message->BodySize)
                || !session.TrySendAsync())
            {
                DisconnectSession(context);
            }
            networkServerReceiver_.Pop(*message);
        }
    }

    MessagePrinter::WriteLine("NetworkServerChannel Receiver 스레드 종료");
}

void NetworkServer::AcceptAsync()
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
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }
    ZeroMemory(acceptContext, sizeof(AcceptContext));
    acceptContext->Context.Type = IOType::Accept;
    acceptContext->SessionId = sessionId;

    const auto clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        ERR_PRINT_VARARGS("Accept 소켓 생성에 실패하였습니다:", WSAGetLastError());
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }

    std::lock_guard lock{ sessionsMutex_ };
    auto [iter, result] = sessions_.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(newSessionId_),
                                            std::forward_as_tuple(clientSocket, newSessionId_));
    auto& session = iter->second;

    if (!session.TryAcceptAsync(listenSocket_, reinterpret_cast<LPOVERLAPPED>(acceptContext)))
    {
        acceptContext->SessionId = NullSessionId;
        DisconnectSession(sessionId);
        mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
        return;
    }
}

void NetworkServer::PostAccept(Session& session)
{
    if (!session.TryPostAccept(iocpHandle_))
    {
        DisconnectSession(session.SessionId);
        return;
    }
    if (!session.TryReceiveAsync())
    {
        DisconnectSession(session.SessionId);
    }
}

void NetworkServer::PostReceive(Session& session, const size_t bytesTransferred)
{
    session.PostReceive(bytesTransferred);

    if (bytesTransferred == 0)
    {
        DisconnectSession(session.SessionId);
        return;
    }

    while (session.TryPopMessage([&](auto sessionId, auto messageType, auto bodySize, auto body)
                                 {
                                     return gameServerSender_.TryPush(sessionId, messageType, bodySize, body);
                                 },
                                 [&](auto sessionId)
                                 {
                                     MessagePrinter::WriteErrorLine("NetworkServer -> GameServer 메시지 큐 Push에 실패하였습니다:", sessionId);
                                     mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
                                 }));
    if (!session.TryReceiveAsync())
    {
        DisconnectSession(session.SessionId);
    }
}

void NetworkServer::PostSend(Session& session, const size_t bytesTransferred)
{
    session.PostSend(bytesTransferred);
    if (bytesTransferred == 0
        || !session.TrySendAsync())
    {
        DisconnectSession(session.SessionId);
        return;
    }
}

void NetworkServer::DisconnectSession(const uint32_t sessionId)
{
    std::lock_guard lock{ sessionsMutex_ };

    if (!sessions_.contains(sessionId))
    {
        return;
    }
    Session& session{ sessions_.at(sessionId) };

    session.Close();
    if (session.IsErasable())
    {
        sessions_.erase(sessionId);
    }
}


