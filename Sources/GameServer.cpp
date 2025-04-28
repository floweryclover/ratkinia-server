//
// Created by floweryclover on 2025-05-01.
//

#include "GameServer.h"
#include "MainServer.h"
#include "Errors.h"

GameServer::GameServer(MpscReceiver<GameServerPipe> gameServerReceiver,
                       MpscSender<MainServerPipe> mainServerSender,
                       MpscSender<NetworkServerPipe> networkServerSender)
    : gameServerReceiver_{ std::move(gameServerReceiver) },
      mainServerSender_{ std::move(mainServerSender) },
      networkServerSender_{ std::move(networkServerSender) },
      ctsHandler_{ *this }
{
}

GameServer::~GameServer()
{
    if (thread_.joinable())
    {
        thread_.join();
    }
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
        if (gameServerReceiver_.Closed())
        {
            break;
        }

        GameServerPipe::PipeMessage message;

        while (gameServerReceiver_.TryPeek(message))
        {
            ctsHandler_.HandleCts(message.SessionId,
                                  message.MessageType,
                                  message.BodySize,
                                  message.Body);
            gameServerReceiver_.Pop();
        }

        gameServerReceiver_.Wait();
    }
}
