//
// Created by floweryclover on 2025-04-02.
//

#include "MainServer.h"

using namespace RatkiniaServer;

MainServer::MainServer()
    : shouldTerminate_{false}
{
}

const std::string& MainServer::Run()
{
    shouldTerminate_.wait(false, std::memory_order_acquire);
    return terminateReason_;
}

void MainServer::RequestTerminate(int code, std::string reason)
{
    bool expected = false;
    if (!shouldTerminate_.compare_exchange_strong(expected, true, std::memory_order_relaxed))
    {
        return;
    }

    terminateReason_ = std::move(reason);
    shouldTerminate_.notify_all();
}
