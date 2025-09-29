//
// Created by floweryclover on 2025-03-31.
//

#include "NetworkServer.h"
#include "ErrorMacros.h"
#include <openssl/ssl.h>
#include <WS2tcpip.h>
#include <mswsock.h>

NetworkServer::NetworkServer(const uint32_t initialAssociatedActor,
                             std::function<void(uint32_t, uint32_t, uint16_t, uint16_t, const char*)> pushMessage,
                             const char* const listenAddress,
                             unsigned short listenPort,
                             const uint32_t acceptPoolSize,
                             const char* const sslCertificateFile,
                             const char* const sslPrivateKeyFile)
    : InitialAssociatedActor{initialAssociatedActor},
      PushMessage{std::move(pushMessage)},
      Iocp{CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0)},
      ListenSocket{socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)},
      SslCtx{SSL_CTX_new(TLS_server_method())},
      AcceptPoolSize{acceptPoolSize},
      AcceptPool{std::make_unique<AcceptOverlappedEx[]>(AcceptPoolSize)},
      newSessionId_{0}
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
        PrepareAcceptSlot(AcceptPool[i]);
    }
}

NetworkServer::~NetworkServer()
{
    CancelIoEx(reinterpret_cast<HANDLE>(ListenSocket), nullptr);
    CRASH_NOW_MSG("Unimplemented");
}

// ReSharper disable once CppMemberFunctionMayBeConst
void NetworkServer::PrepareAcceptSlot(AcceptOverlappedEx& slot)
{
    slot.ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CRASH_COND(slot.ClientSocket == INVALID_SOCKET);

    slot.IoType = AcceptOverlappedEx::IoType_Accept;

    const auto ssl = SSL_new(SslCtx);
    CRASH_COND(ssl == nullptr);
    CRASH_COND(1 != SSL_set_fd(ssl, slot.ClientSocket));
    CRASH_COND_MSG(AcceptEx(ListenSocket,
                slot.ClientSocket,
                slot.AddressBuffer,
                0,
                sizeof(SOCKADDR_IN) + 16,
                sizeof(SOCKADDR_IN) + 16,
                nullptr,
                reinterpret_cast<LPOVERLAPPED>(&slot)) == FALSE &&
                GetLastError() != ERROR_IO_PENDING, GetLastError());
}

void NetworkServer::WorkerThreadBody(const int threadId)
{
    std::osyncstream{std::cout} << "IOCP 워커 스레드 " << threadId << " 시작" << std::endl;
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
            CRASH_COND_MSG(PostQueuedCompletionStatus(Iocp, 0, 0, nullptr) == FALSE, GetLastError());
            break;
        }

        CRASH_COND_MSG(overlapped == nullptr, GetLastError());
        const uint8_t ioType = reinterpret_cast<AcceptOverlappedEx*>(overlapped)->IoType;
        if (ioType == AcceptOverlappedEx::IoType_Accept)
        {
            if (success)
            {

            }
            else
            {

            }
            continue;
        }

        if (!success)
        {
            const OverlappedEx& overlappedEx = *reinterpret_cast<OverlappedEx*>(overlapped);
            Session& session = *reinterpret_cast<Session*>(completionKey);
            const uint32_t context = session.Context;
            if (overlappedEx.IoType == Session::IoType_Send)
            {
                if (GetLastError() != ERROR_NETNAME_DELETED)
                {
                    ERR_PRINT_VARARGS("세션 ", session.Context, " 송신 에러: ", GetLastError());
                }
            }
            else if (overlappedEx.IoType == Session::IoType_Receive)
            {
                if (GetLastError() != ERROR_NETNAME_DELETED)
                {
                    ERR_PRINT_VARARGS("세션 ", session.Context, " 수신 에러: ", GetLastError());
                }
            }
            else // TODO: Accept는 CompletionKey가 Session이 아님
            {
                ERR_PRINT_VARARGS("세션 ", session.Context, " 연결 수립 에러: ", GetLastError());
            }
            session.PostIoFailed(overlappedEx.IoType);
            SessionCleanupRoutine(context);
            continue;
        }

        Session& session = *reinterpret_cast<Session*>(completionKey);
        const OverlappedEx& context = *reinterpret_cast<OverlappedEx*>(overlapped);
        if (context.IoType == Session::IoType_Accept)
        {
            PostAccept(session);
        }
        else if (context.IoType == Session::IoType_Receive)
        {
            // TODO: WSARecv()를 호출해야 0(FIN)패킷이 받아지는지
            PostReceive(session, bytesTransferred);
        }
        else
        {
            PostSend(session, bytesTransferred);
        }
    }
    std::osyncstream{std::cout} << "IOCP 워커 스레드 " << threadId << " 종료" << std::endl;
}

void NetworkServer::PostAccept(AcceptOverlappedEx& slot)
{
    int noDelay = 1;
    CRASH_COND_MSG(SOCKET_ERROR == setsockopt(
                       Socket,
                       IPPROTO_TCP,
                       TCP_NODELAY,
                       reinterpret_cast<char*>(&noDelay),
                       sizeof(noDelay)),
                   WSAGetLastError());

    if (SSL_accept(Ssl) != 1)
    {
        char buf[256];
        ERR_PRINT_VARARGS("세션 ", Context, " SSL_accept() 에러: ", ERR_error_string(ERR_get_error(), buf));
        return false;
    }
    SSL_set_bio(Ssl, ReadBio, WriteBio);

    if (nullptr
        == CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket),
                                  Iocp,
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

    GetAcceptExSockaddrs(ReceiveTlsBuffer.get(),
                         0,
                         sizeof(SOCKADDR_IN) + 16,
                         sizeof(SOCKADDR_IN) + 16,
                         reinterpret_cast<sockaddr**>(&localAddr),
                         &localAddrLen,
                         reinterpret_cast<sockaddr**>(&remoteAddr),
                         &remoteAddrLen);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &remoteAddr->sin_addr, buf, INET_ADDRSTRLEN))
    {
        address_ = buf;
    }
    else
    {
        address_ = "알 수 없음";
    }
}

void NetworkServer::PostReceive(Session& session, const size_t bytesTransferred)
{
    const uint32_t context = session.Context;
    const bool succeeded = [&]
    {
        std::shared_lock lock{sessionsMutex_};
        return session.TryPostReceive(bytesTransferred) &&
            bytesTransferred > 0 &&
            session.TryReceiveAsync();
    }();

    if (!succeeded)
    {
        SessionCleanupRoutine(context);
    }

    /// TODO Push to MPSC queue
}

void NetworkServer::PostSend(Session& session, const size_t bytesTransferred)
{
    const uint32_t context = session.Context;
    const bool succeeded = [&]
    {
        std::shared_lock lock{sessionsMutex_};
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
    if (std::shared_lock lock{sessionsMutex_};
        !sessions_.contains(context) ||
        !sessions_.at(context)->Close())
    {
        return;
    }

    if (std::unique_lock lock{sessionsMutex_};
        sessions_.contains(context))
    {
        sessions_.erase(context);
    }
}
