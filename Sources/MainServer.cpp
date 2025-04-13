//
// Created by floweryclover on 2025-04-02.
//

#include "MainServer.h"

using namespace RatkiniaServer;

MainServer::MainServer()
    : shouldTerminate_{ false }
{
}

void MainServer::Run()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        {
            std::lock_guard<std::mutex> lock{ terminateOperationMutex_ };
            if (shouldTerminate_)
            {
                break;
            }
        }
    }
}

void MainServer::RequestTerminate()
{
    std::lock_guard<std::mutex> lock{ terminateOperationMutex_ };
    shouldTerminate_ = true;
}
