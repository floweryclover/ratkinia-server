//
// Created by floweryclover on 2025-03-31.
//

#ifndef IOSERVER_H
#define IOSERVER_H

#include <ws2tcpip.h>
#include <WinSock2.h>
#include <thread>
#include <string>
#include <vector>

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
        MainServer& mainServer_;
        std::thread listenThread_;
        std::vector<std::thread> workerThreads_;
        SOCKET listenSocket_;

        void WorkerThreadBody(int threadId);

        void ListenThreadBody();
    };
}

#endif //IOSERVER_H
