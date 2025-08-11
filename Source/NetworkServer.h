//
// Created by floweryclover on 2025-03-31.
//

#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "SessionReceiveBuffersView.h"
#include "Session.h"
#include <thread>
#include <vector>
#include <memory>
#include <unordered_map>

class NetworkServer final
{
public:
    explicit NetworkServer(const char* listenAddress,
                           unsigned short listenPort,
                           uint32_t acceptPoolSize,
                           const char* sslCertificateFile,
                           const char* sslPrivateKeyFile);

    ~NetworkServer();

    NetworkServer(const NetworkServer&) = delete;

    NetworkServer& operator=(const NetworkServer&) = delete;

    NetworkServer(NetworkServer&&) = delete;

    NetworkServer& operator=(NetworkServer&&) = delete;

    std::optional<uint32_t> TryClearClosedSession();

    void PrepareAcceptPool();

    [[nodiscard]]
    SessionReceiveBuffersView SessionReceiveBuffers()
    {
        return SessionReceiveBuffersView{sessions_};
    }

    template<typename TMessage>
    void SendMessage(const uint32_t context, const uint16_t messageType, TMessage&& message)
    {
        if (!sessions_.contains(context))
        {
            return;
        }

        Session& session = sessions_.at(context);

        if (!session.TryPushMessage(messageType, std::forward<TMessage>(message))
            || !session.TrySendAsync())
        {
            CloseSession(session);
        }
    }


private:
    struct AcceptContext final
    {
        OverlappedEx Context;
        Session* Session;
    };

    const HANDLE Iocp;
    const SOCKET ListenSocket;
    SSL_CTX* const SslCtx;
    const uint32_t AcceptPoolSize;
    std::atomic_uint32_t acceptingSessionsCount_;

    uint32_t newSessionId_;
    std::unordered_map<uint32_t, Session> sessions_;
    std::unique_ptr<AcceptContext[]> acceptContexts_;

    std::vector<std::thread> workerThreads_;

    std::mutex pendingSessionsToEraseMutex_;
    std::vector<uint32_t> pendingSessionsToErase_;

    void WorkerThreadBody(int threadId);

    void PostAccept(Session& session);

    void PostReceive(Session& session, size_t bytesTransferred);

    void PostSend(Session& session, size_t bytesTransferred);

    void CloseSession(Session& session);
};

#endif //NETWORKSERVER_H
