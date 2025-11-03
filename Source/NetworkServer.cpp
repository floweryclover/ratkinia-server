//
// Created by floweryclover on 2025-03-31.
//

#include "NetworkServer.h"
#include "ActorMessageDispatcher.h"
#include "ErrorMacros.h"
#include "Msg_Cts.h"
#include "Msg_SessionDisconnected.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <WS2tcpip.h>
#include <mswsock.h>

NetworkServer::NetworkServer(std::string initialAssociatedActor,
                             ActorMessageDispatcher& actorMessageDispatcher,
                             const char* const listenAddress,
                             unsigned short listenPort,
                             const uint32_t acceptPoolSize,
                             const char* const sslCertificateFile,
                             const char* const sslPrivateKeyFile)
    : InitialAssociatedActor{ std::move(initialAssociatedActor) },
      Iocp{ CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0) },
      ListenSocket{ socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) },
      SslCtx{ SSL_CTX_new(TLS_server_method()) },
      AcceptPoolSize{ acceptPoolSize },
      AcceptPool{ std::make_unique<OverlappedEx[]>(AcceptPoolSize) },
      actorMessageDispatcher_{ actorMessageDispatcher },
      newContext_{ 0 }
{
    CRASH_COND(Iocp == nullptr);
    CRASH_COND(ListenSocket == INVALID_SOCKET);
    CRASH_COND(SslCtx == nullptr);

    CRASH_COND(1 != SSL_CTX_use_certificate_file(SslCtx, sslCertificateFile, SSL_FILETYPE_PEM));
    CRASH_COND(1 != SSL_CTX_use_PrivateKey_file(SslCtx, sslPrivateKeyFile, SSL_FILETYPE_PEM));
    CRASH_COND(1 != SSL_CTX_check_private_key(SslCtx));

    SOCKADDR_IN sockaddrIn{};
    sockaddrIn.sin_port = htons(listenPort);
    sockaddrIn.sin_family = AF_INET;
    CRASH_COND_MSG(1 != inet_pton(AF_INET, listenAddress, &sockaddrIn.sin_addr), WSAGetLastError());

    CRASH_COND_MSG(nullptr == CreateIoCompletionPort(reinterpret_cast<HANDLE>(ListenSocket), Iocp, 0,0),
                   GetLastError());

    int reuseAddr = 1;
    CRASH_COND_MSG(SOCKET_ERROR == setsockopt(
                       ListenSocket,
                       SOL_SOCKET,
                       SO_REUSEADDR,
                       reinterpret_cast<char*>(&reuseAddr),
                       sizeof(reuseAddr)),
                   WSAGetLastError());

    CRASH_COND_MSG(SOCKET_ERROR == bind(
                       ListenSocket,
                       reinterpret_cast<sockaddr*>(&sockaddrIn),
                       sizeof(SOCKADDR_IN)),
                   WSAGetLastError());

    CRASH_COND_MSG(SOCKET_ERROR == listen(ListenSocket, AcceptPoolSize), WSAGetLastError());

    const auto threadCount = std::thread::hardware_concurrency();
    for (int i = 0; i < threadCount; ++i)
    {
        workerThreads_.emplace_back(&NetworkServer::WorkerThreadBody, this, i);
    }

    for (int i = 0; i < AcceptPoolSize; ++i)
    {
        AcceptPool[i].Data.emplace<OverlappedEx::AcceptData>();
        PrepareAcceptSlot(AcceptPool[i]);
    }
}

NetworkServer::~NetworkServer()
{
    CRASH_COND_MSG(PostQueuedCompletionStatus(Iocp, 0, 0, nullptr) == FALSE, GetLastError());
    for (auto& workerThread : workerThreads_)
    {
        workerThread.join();
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void NetworkServer::PrepareAcceptSlot(OverlappedEx& slot)
{
    auto& acceptData = std::get<OverlappedEx::AcceptData>(slot.Data);
    acceptData.ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CRASH_COND(acceptData.ClientSocket == INVALID_SOCKET);
    CRASH_COND_MSG(AcceptEx(ListenSocket,
                       acceptData.ClientSocket,
                       acceptData.AddressBuffer,
                       0,
                       sizeof(SOCKADDR_IN) + 16,
                       sizeof(SOCKADDR_IN) + 16,
                       nullptr,
                       reinterpret_cast<LPOVERLAPPED>(&slot)) == FALSE &&
                   GetLastError() != ERROR_IO_PENDING,
                   GetLastError());
}

void NetworkServer::WorkerThreadBody(const int threadId)
{
    std::osyncstream{ std::cout } << "IOCP 워커 스레드 " << threadId << " 시작" << std::endl;

    while (true)
    {
        DWORD bytesTransferred = 0;
        LPOVERLAPPED overlapped = nullptr;
        ULONG_PTR completionKey_unused = 0;
        const bool success = GetQueuedCompletionStatus(Iocp,
                                                       &bytesTransferred,
                                                       &completionKey_unused,
                                                       &overlapped,
                                                       INFINITE);
        if (!overlapped) // 종료 명령
        {
            CRASH_COND_MSG(PostQueuedCompletionStatus(Iocp, 0, 0, nullptr) == FALSE, GetLastError());
            break;
        }

        CRASH_COND_MSG(overlapped == nullptr, GetLastError());
        OverlappedEx& overlappedEx = *reinterpret_cast<OverlappedEx*>(overlapped);
        if (std::holds_alternative<OverlappedEx::AcceptData>(overlappedEx.Data))
        {
            if (success)
            {
                PostAccept(overlappedEx);
            }
            else
            {
                ERR_PRINT_VARARGS("연결 수립 중 에러: ", GetLastError());
            }
            PrepareAcceptSlot(overlappedEx);
            continue;
        }

        auto& ioData = std::get<OverlappedEx::IoData>(overlappedEx.Data);
        if (!success)
        {
            if (ioData.IoType == Session::IoType_Send)
            {
                if (GetLastError() != ERROR_NETNAME_DELETED && GetLastError() != ERROR_CONNECTION_ABORTED)
                {
                    ERR_PRINT_VARARGS("세션 ", ioData.Session->Context, " 송신 에러: ", GetLastError());
                }
            }
            else if (ioData.IoType == Session::IoType_Receive)
            {
                if (GetLastError() != ERROR_NETNAME_DELETED && GetLastError() != ERROR_CONNECTION_ABORTED)
                {
                    ERR_PRINT_VARARGS("세션 ", ioData.Session->Context, " 수신 에러: ", GetLastError());
                }
            }
            else
            {
                CRASH_NOW();
            }

            const uint32_t context = ioData.Session->Context;
            ioData.Session->PostIoFailed(ioData.IoType);
            SessionCleanupRoutine(context);
            continue;
        }

        if (ioData.IoType == Session::IoType_Send)
        {
            PostSend(*ioData.Session, bytesTransferred);
        }
        else if (ioData.IoType == Session::IoType_Receive)
        {
            PostReceive(*ioData.Session, bytesTransferred);
        }
        else
        {
            std::osyncstream{ std::cout } << "Hi" << std::endl;
            std::shared_lock lock{ sessionsMutex_ };
            ioData.Session->PostInitiateSend();
            if (!ioData.Session->TrySendAsync())
            {
                const uint32_t context = ioData.Session->Context;
                SessionCleanupRoutine(context);
            }
        }
    }
    std::osyncstream{ std::cout } << "IOCP 워커 스레드 " << threadId << " 종료" << std::endl;
}

void NetworkServer::PostAccept(OverlappedEx& slot)
{
    auto& acceptData = std::get<OverlappedEx::AcceptData>(slot.Data);

    const auto ssl = SSL_new(SslCtx);
    CRASH_COND(ssl == nullptr);
    if (SSL_set_fd(ssl, acceptData.ClientSocket) != 1)
    {
        char buf[256];
        CRASH_NOW_MSG(ERR_error_string(ERR_get_error(), buf));
    }

    if (SSL_accept(ssl) != 1)
    {
        char buf[256];
        ERR_PRINT_VARARGS("SSL_accept() 에러: ", ERR_error_string(ERR_get_error(), buf));
        shutdown(acceptData.ClientSocket, SD_BOTH);
        closesocket(acceptData.ClientSocket);
        return;
    }

    int noDelay = 1;
    CRASH_COND_MSG(setsockopt(
                       acceptData.ClientSocket,
                       IPPROTO_TCP,
                       TCP_NODELAY,
                       reinterpret_cast<char*>(&noDelay),
                       sizeof(noDelay)) == SOCKET_ERROR,
                   WSAGetLastError());

    CRASH_COND_MSG(CreateIoCompletionPort(reinterpret_cast<HANDLE>(acceptData.ClientSocket),
                       Iocp,
                       reinterpret_cast<ULONG_PTR>(this),
                       0) == nullptr,
                   GetLastError());

    SOCKADDR_IN* localAddr,* remoteAddr;
    int localAddrLen, remoteAddrLen;
    GetAcceptExSockaddrs(acceptData.AddressBuffer,
                         0,
                         sizeof(SOCKADDR_IN) + 16,
                         sizeof(SOCKADDR_IN) + 16,
                         reinterpret_cast<sockaddr**>(&localAddr),
                         &localAddrLen,
                         reinterpret_cast<sockaddr**>(&remoteAddr),
                         &remoteAddrLen);

    char buf[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &remoteAddr->sin_addr, buf, INET_ADDRSTRLEN))
    {
        ERR_PRINT_VARARGS("주소 식별 불가: ", WSAGetLastError());
        SSL_shutdown(ssl);
        SSL_free(ssl);
        shutdown(acceptData.ClientSocket, SD_BOTH);
        closesocket(acceptData.ClientSocket);
        return;
    }

    const uint32_t context = newContext_.fetch_add(1, std::memory_order_relaxed);
    auto session = std::make_unique<Session>(acceptData.ClientSocket, buf, ssl, context, InitialAssociatedActor);

    const auto emplacedSession = [&]
    {
        std::unique_lock lock{ sessionsMutex_ };
        const auto [sessionIter, emplacedSession] = sessions_.try_emplace(context, std::move(session));
        return emplacedSession ? sessionIter->second.get() : nullptr;
    }();

    CRASH_COND(emplacedSession == nullptr);
    if (!emplacedSession->TryReceiveAsync())
    {
        SessionCleanupRoutine(context);
    }
}

void NetworkServer::PostReceive(Session& session, const size_t bytesTransferred)
{
    const uint32_t context = session.Context;
    const bool succeeded = [&]
    {
        std::shared_lock lock{ sessionsMutex_ };

        const bool returnValue = session.TryPostReceive(bytesTransferred) &&
                                 bytesTransferred > 0 &&
                                 session.TryReceiveAsync();

        while (const auto message = session.PeekReceivedMessage())
        {
            auto ownedBody = std::make_unique<char[]>(message->BodySize);
            memcpy(ownedBody.get(), message->Body, message->BodySize);

            auto msg_cts = std::make_unique<Msg_Cts>();
            msg_cts->Context = context;
            msg_cts->MessageType = message->MessageType;
            msg_cts->BodySize = message->BodySize;
            msg_cts->Body = std::move(ownedBody);

            CRASH_COND(!actorMessageDispatcher_.TryPushMessageTo(session.AssociatedActor, std::move(msg_cts)));
            session.PopReceivedMessage(RatkiniaProtocol::MessageHeaderSize + message->BodySize);
        }

        return returnValue;
    }();

    if (!succeeded)
    {
        SessionCleanupRoutine(context);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void NetworkServer::InitiateSend(Session& session)
{
    if (const auto overlapped = session.InitiateSendAsync())
    {
        const auto result = PostQueuedCompletionStatus(Iocp, 0, 0, overlapped);
        CRASH_COND_MSG(result == FALSE, GetLastError());
    }
}

void NetworkServer::PostSend(Session& session, const size_t bytesTransferred)
{
    std::osyncstream{ std::cout } << bytesTransferred << std::endl;
    const uint32_t context = session.Context;
    const bool succeeded = [&]
    {
        std::shared_lock lock{ sessionsMutex_ };
        return session.TryPostSend(bytesTransferred) &&
               session.TrySendAsync();
    }();

    if (!succeeded)
    {
        SessionCleanupRoutine(context);
    }
}

void NetworkServer::SessionCleanupRoutine(const uint32_t context)
{
    {
        std::shared_lock lock{ sessionsMutex_ };
    }
    if (const auto iter = sessions_.find(context);
        iter == sessions_.end() || !iter->second->Close())
    {
        return;
    }

    const auto assosiatedActor = [&]() -> std::optional<std::string>
    {
        std::unique_lock lock{ sessionsMutex_ };
        const auto iter = sessions_.find(context);
        if (iter == sessions_.end())
        {
            return std::nullopt;
        }
        auto actor = std::move(iter->second->AssociatedActor);
        sessions_.erase(iter);
        return std::move(actor);
    }();

    if (assosiatedActor)
    {
        auto msg_sessionDisconnected = std::make_unique<Msg_SessionDisconnected>();
        msg_sessionDisconnected->Context = context;
        CRASH_COND(!actorMessageDispatcher_.TryPushMessageTo(*assosiatedActor, std::move(msg_sessionDisconnected)));
    }
}
