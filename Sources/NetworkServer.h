//
// Created by floweryclover on 2025-03-31.
//

#ifndef IOSERVER_H
#define IOSERVER_H

#include "Session.h"
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <thread>
#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <unordered_map>

namespace RatkiniaServer
{
    class MainServer;

    class NetworkServer final
    {
    public:
        explicit NetworkServer(MainServer& mainServer);

        ~NetworkServer();

        void Start(const std::string& listenAddress, unsigned short listenPort);

    private:
        struct AcceptContext final
        {
            uint64_t SessionId {};
            OverlappedEx Context {};
        };

        static constexpr int AcceptPoolSize = 8;
        static constexpr size_t SessionBufferSize = 1024;

        MainServer& mainServer_;
        SOCKET listenSocket_;
        HANDLE iocpHandle_;
        std::vector<std::thread> workerThreads_;
        std::atomic_bool shouldStop_;

        uint64_t newSessionId_;
        std::unordered_map<uint64_t, Session> sessions_;
        std::unique_ptr<AcceptContext[]> acceptContexts_;

        void WorkerThreadBody(int threadId);

        bool AcceptAsync(AcceptContext& acceptContext);
    };
}

#endif //IOSERVER_H
