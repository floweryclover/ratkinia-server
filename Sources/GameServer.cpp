//
// Created by floweryclover on 2025-05-01.
//

#include "GameServer.h"
#include "Errors.h"
#include "CTS.pb.h"
#include "MpscMessageQueue.h"

GameServer::GameServer(MpscMessageQueue& messageQueue)
    : messageQueue_{ messageQueue }
{

}

void GameServer::Start()
{
    ERR_FAIL_COND(thread_.joinable());

    thread_ = std::thread{ &GameServer::ThreadBody, this };
}

void GameServer::ThreadBody()
{
    while (true)
    {
        uint64_t sessionId;
        uint16_t messageType;
        uint16_t bodySize;
        const char* body;

        while (messageQueue_.TryPeek(sessionId, messageType, bodySize, body))
        {
            HandleMessage(sessionId, messageType, bodySize, body);

            messageQueue_.Pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void GameServer::HandleMessage(uint64_t sessionId,
                               uint16_t messageType,
                               uint16_t bodySize,
                               const char* body)
{
    using namespace RatkiniaProtocol::Cts;

    switch (messageType)
    {
        case static_cast<int>(MessageType::LoginRequest):
        {
            LoginRequest loginRequest;
            loginRequest.ParseFromArray(body, bodySize);
        }
        default:
        {
            ERR_PRINT_VARARGS("처리되지 않은 메시지:", messageType);
            break;
        }
    }
}
