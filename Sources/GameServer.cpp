//
// Created by floweryclover on 2025-05-01.
//

#include "GameServer.h"
#include "MainServer.h"
#include "Errors.h"
#include "GameServerTerminal.h"

GameServer::GameServer(MainServer& mainServer)
    : mainServer_{ mainServer },
      messageQueue_{},
      ctsHandler_{ *this }
{
}

GameServer::~GameServer() = default;

void GameServer::Start()
{
    ERR_FAIL_COND(thread_.joinable());

    thread_ = std::thread{ &GameServer::ThreadBody, this };
}

void GameServer::ThreadBody()
{
    while (true)
    {
        messageQueue_.SwapQueue();

        uint64_t sessionId;
        uint16_t messageType;
        uint16_t bodySize;
        const char* body;

        while (messageQueue_.TryPeekMessage(sessionId, messageType, bodySize, body))
        {
            ctsHandler_.HandleCts(sessionId, messageType, bodySize, body);
            messageQueue_.PopMessage();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void GameServer::DisconnectSession(uint64_t session)
{

}

void GameServer::Terminate()
{
    mainServer_.RequestTerminate();
}

