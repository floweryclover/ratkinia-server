//
// Created by floweryclover on 2025-03-31.
//

#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "Session.h"
#include <absl/container/flat_hash_map.h>
#include <thread>
#include <vector>
#include <memory>
#include <shared_mutex>

class ActorMessageDispatcher;
using HANDLE = void*;

class alignas(64) NetworkServer final
{
public:
    explicit NetworkServer(std::string initialAssociatedActor,
                           ActorMessageDispatcher& actorMessageDispatcher,
                           const char* listenAddress,
                           unsigned short listenPort,
                           uint32_t acceptPoolSize,
                           const char* sslCertificateFile,
                           const char* sslPrivateKeyFile);

    ~NetworkServer();

    NetworkServer(const NetworkServer&) = delete;

    NetworkServer& operator=(const NetworkServer&) = delete;

    NetworkServer(NetworkServer&&) = delete;

    NetworkServer& operator=(NetworkServer&&) = delete;

    void DisconnectSession(const uint32_t context)
    {
        SessionCleanupRoutine(context);
    }

    template<typename TProtobufMessage>
    void SendMessageTo(const uint32_t context, const uint16_t messageType, const TProtobufMessage& message)
    {
        std::shared_lock lock{ sessionsMutex_ };
        const auto session = sessions_.find(context);
        if (session == sessions_.end())
        {
            return;
        }
        session->second->PushMessage(messageType, message);
        InitiateSend(*session->second.get());
    }

private:
    const std::string InitialAssociatedActor;
    const HANDLE Iocp;
    const SOCKET ListenSocket;
    SSL_CTX* const SslCtx;

    const uint32_t AcceptPoolSize;
    const std::unique_ptr<OverlappedEx[]> AcceptPool;

    ActorMessageDispatcher& actorMessageDispatcher_;

    alignas(64) std::atomic_uint32_t newContext_;

    std::shared_mutex sessionsMutex_;
    absl::flat_hash_map<uint32_t, std::unique_ptr<Session>> sessions_;

    std::vector<std::thread> workerThreads_;

    void PrepareAcceptSlot(OverlappedEx& slot);

    void WorkerThreadBody(int threadId);

    void PostAccept(OverlappedEx& slot);

    void PostReceive(Session& session, size_t bytesTransferred);

    void InitiateSend(Session& session);

    void PostSend(Session& session, size_t bytesTransferred);

    void SessionCleanupRoutine(uint32_t context);
};

#endif //NETWORKSERVER_H
