//
// Created by floweryclover on 2025-04-02.
//

#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <string>
#include <mutex>

namespace RatkiniaServer
{
    class MainServer final
    {
    public:
        explicit MainServer();

        void Run();

        void RequestTerminate();

    private:
        std::mutex terminateOperationMutex_;
        bool shouldTerminate_;
    };
}

#endif //MAINSERVER_H
