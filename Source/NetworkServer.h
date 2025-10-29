//
// Created by floweryclover on 2025-03-31.
//

#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "Session.h"
#include <absl/container/flat_hash_map.h>
#include <functional>
#include <thread>
#include <vector>
#include <memory>
#include <shared_mutex>

using HANDLE = void*;

class alignas(64) NetworkServer final
{
public:
    explicit NetworkServer(uint32_t initialAssociatedActor,
                           std::function<void(uint32_t, uint32_t, uint16_t, uint16_t, const char*)> pushMessage,
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

private:
    const uint32_t InitialAssociatedActor;
    const std::function<void(uint32_t, uint32_t, uint16_t, uint16_t, const char*)> PushMessage;

    const HANDLE Iocp;
    const SOCKET ListenSocket;
    SSL_CTX* const SslCtx;

    const uint32_t AcceptPoolSize;
    const std::unique_ptr<OverlappedEx[]> AcceptPool;

    alignas(64) std::atomic_uint32_t newContext_;

    std::shared_mutex sessionsMutex_;
    absl::flat_hash_map<uint32_t, std::unique_ptr<Session>> sessions_;

    std::vector<std::thread> workerThreads_;

    void PrepareAcceptSlot(OverlappedEx& slot);

    void WorkerThreadBody(int threadId);

    void PostAccept(OverlappedEx& slot);

    void PostReceive(Session& session, size_t bytesTransferred);

    void PostSend(Session& session, size_t bytesTransferred);

    void SessionCleanupRoutine(uint32_t context);
};

#endif //NETWORKSERVER_H
