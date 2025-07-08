//
// Created by floweryclover on 2025-05-01.
//

#include "GameServer.h"
#include "MainServer.h"
#include "Errors.h"

GameServer::GameServer(GameServerChannel::MpscReceiver gameServerReceiver,
                       MainServerChannel::MpscSender mainServerSender,
                       NetworkServerChannel::SpscSender networkServerSender)
    : gameServerReceiver_{ std::move(gameServerReceiver) },
      mainServerSender_{ std::move(mainServerSender) },
      ctsStub_{ *this },
      stcProxy_{ std::move(networkServerSender) },
      dbConnection_{ DbHost }
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
        if (gameServerReceiver_.IsClosed())
        {
            break;
        }

        while (const auto message = gameServerReceiver_.TryPeek())
        {
            ctsStub_.HandleCts(message->Context,
                               message->MessageType,
                               message->BodySize,
                               message->Body);
            gameServerReceiver_.Pop(*message);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{ 16 });
    }
}

void GameServer::ShouldTerminate()
{
    mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
}
