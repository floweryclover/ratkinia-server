//
// Created by floweryclover on 2025-03-31.
//

#include "NetworkServer.h"
#include "MessagePrinter.h"
#include "ErrorMacros.h"
#include <openssl/ssl.h>
#include <WS2tcpip.h>
#include <ranges>

NetworkServer::NetworkServer(const char* const listenAddress,
                             unsigned short listenPort,
                             const uint32_t acceptPoolSize,
                             const char* const sslCertificateFile,
                             const char* const sslPrivateKeyFile)
    : Iocp{ CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0) },
      ListenSocket{ socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) },
      SslCtx{ SSL_CTX_new(TLS_server_method()) },
      AcceptPoolSize{ acceptPoolSize },
      newSessionId_{ 0 },
      acceptContexts_{ std::make_unique<AcceptContext[]>(AcceptPoolSize) }
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

    CRASH_COND_MSG(nullptr == CreateIoCompletionPort(reinterpret_cast<HANDLE>(ListenSocket),
                       Iocp,
                       reinterpret_cast<ULONG_PTR>(this),
                       0),
                   GetLastError());

    int reuseAddr = 1;
    CRASH_COND_MSG(SOCKET_ERROR == setsockopt(
                       ListenSocket,
                       SOL_SOCKET,
                       SO_REUSEADDR,
                       reinterpret_cast<char*>(&reuseAddr),
                       sizeof(reuseAddr)),
                   WSAGetLastError());

    int nodelay = 1;
    CRASH_COND_MSG(SOCKET_ERROR == setsockopt(
                       ListenSocket,
                       IPPROTO_TCP,
                       TCP_NODELAY,
                       reinterpret_cast<char*>(&nodelay),
                       sizeof(nodelay)),
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

    ZeroMemory(acceptContexts_.get(), sizeof(AcceptContext) * AcceptPoolSize);
    PrepareAcceptPool();
}

NetworkServer::~NetworkServer()
{
    CRASH_COND(0 == PostQueuedCompletionStatus(Iocp, 0, 0, nullptr));

    for (auto& workerThread : workerThreads_)
    {
        workerThread.join();
    }

    for (auto& session : sessions_ | std::views::values)
    {
        session.Close();
    }

    SSL_CTX_free(SslCtx);
    closesocket(ListenSocket);
    CloseHandle(Iocp);
}

std::optional<uint32_t> NetworkServer::TryClearClosedSession()
{
    std::lock_guard lock{ pendingSessionsToEraseMutex_ };
    if (pendingSessionsToErase_.empty())
    {
        return std::nullopt;
    }

    const uint32_t context = pendingSessionsToErase_.back();
    sessions_.erase(context);
    pendingSessionsToErase_.pop_back();
    return context;
}

void NetworkServer::PrepareAcceptPool()
{
    while (acceptingSessionsCount_.load(std::memory_order_relaxed) < AcceptPoolSize)
    {
        while (sessions_.contains(newSessionId_))
        {
            newSessionId_ += 1;
        }

        AcceptContext* availableAcceptContext = nullptr;
        for (int i = 0; i < AcceptPoolSize; ++i)
        {
            if (acceptContexts_[i].Session == nullptr)
            {
                availableAcceptContext = &acceptContexts_[i];
                break;
            }
        }

        CRASH_COND(availableAcceptContext == nullptr);
        ZeroMemory(availableAcceptContext, sizeof(AcceptContext));
        availableAcceptContext->Context.Type = IOType::Accept;

        const auto newClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        CRASH_COND(newClientSocket == INVALID_SOCKET);

        const auto ssl = SSL_new(SslCtx);
        CRASH_COND(ssl == nullptr);
        CRASH_COND(1 != SSL_set_fd(ssl, newClientSocket));

        auto [iter, result] = sessions_.emplace(std::piecewise_construct,
                                                std::forward_as_tuple(newSessionId_),
                                                std::forward_as_tuple(Iocp, newClientSocket, ssl, newSessionId_));
        auto& session = iter->second;
        availableAcceptContext->Session = &session;

        CRASH_COND(!session.TryAcceptAsync(ListenSocket, reinterpret_cast<LPOVERLAPPED>(availableAcceptContext)));
        acceptingSessionsCount_.fetch_add(1, std::memory_order_relaxed);
    }
}

void NetworkServer::WorkerThreadBody(const int threadId)
{
    MessagePrinter::WriteLine("IOCP 워커 스레드 ", threadId, " 시작");
    while (true)
    {
        DWORD bytesTransferred = 0;
        LPOVERLAPPED overlapped = nullptr;
        ULONG_PTR completionKey = 0;
        const bool success = GetQueuedCompletionStatus(Iocp,
                                                       &bytesTransferred,
                                                       &completionKey,
                                                       &overlapped,
                                                       INFINITE);
        if (completionKey == 0) // 종료 명령
        {
            break;
        }

        if (!success)
        {
            const OverlappedEx& context = *reinterpret_cast<OverlappedEx*>(overlapped);
            CRASH_COND(context.Type == IOType::Accept);

            Session& session{ *reinterpret_cast<Session*>(completionKey) };
            if (context.Type == IOType::Receive)
            {
                if (GetLastError() != ERROR_NETNAME_DELETED)
                {
                    ERR_PRINT_VARARGS("세션", session.Context, "수신 에러:", GetLastError());
                }
                PostReceive(session, 0);
            }
            else
            {
                if (GetLastError() != ERROR_NETNAME_DELETED)
                {
                    ERR_PRINT_VARARGS("세션", session.Context, "송신 에러:", GetLastError());
                }
                PostSend(session, 0);
            }
            CloseSession(session);
            continue;
        }

        const OverlappedEx& context = *reinterpret_cast<OverlappedEx*>(overlapped);
        if (context.Type == IOType::Accept)
        {
            {
                auto& acceptContext = *reinterpret_cast<AcceptContext*>(reinterpret_cast<size_t>(overlapped));
                PostAccept(*acceptContext.Session);
                acceptContext.Session = nullptr;
            }
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
    MessagePrinter::WriteLine("IOCP 워커 스레드 ", threadId, " 종료");
}

void NetworkServer::PostAccept(Session& session)
{
    if (!session.TryPostAccept() ||
        !session.TryReceiveAsync())
    {
        CloseSession(session);
    }

    acceptingSessionsCount_.fetch_sub(1, std::memory_order_relaxed);
}

void NetworkServer::PostReceive(Session& session, const size_t bytesTransferred)
{
    if (!session.TryPostReceive(bytesTransferred) ||
        bytesTransferred == 0 ||
        !session.TryReceiveAsync())
    {
        CloseSession(session);
    }
}

void NetworkServer::PostSend(Session& session, const size_t bytesTransferred)
{
    if (!session.TryPostSend(bytesTransferred) ||
        session.IsSendPending() && !session.TrySendAsync())
    {
        CloseSession(session);
    }
}

void NetworkServer::CloseSession(Session& session)
{
    session.Close();
    if (session.IsIoClear())
    {
        std::lock_guard lock{ pendingSessionsToEraseMutex_ };
        pendingSessionsToErase_.emplace_back(session.Context);
    }
}
