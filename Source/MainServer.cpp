//
// Created by floweryclover on 2025-05-01.
//

#include "MainServer.h"
#include "NetworkServer.h"
#include "Database.h"

#include "Event_SessionErased.h"

#include <pqxx/pqxx>

using namespace Database;
using namespace pqxx;

MainServer::MainServer(Registrar registrar,
                       std::string listenAddress,
                       const uint16_t listenPort,
                       std::string dbHost,
                       const uint16_t acceptPoolSize,
                       const char* const certificateFile,
                       const char* const privateKeyFile)
    : ListenAddress{ std::move(listenAddress) },
      ListenPort(listenPort),
      DbHost{ std::move(dbHost) },
      networkServer_
      {
          ListenAddress.c_str(),
          ListenPort,
          acceptPoolSize,
          certificateFile,
          privateKeyFile
      },
      executor_{ std::thread::hardware_concurrency(), 1024 },
      stub_{ environment_ },
      proxy_{ networkServer_ },
      dbConnection_{ DbHost },
      componentManager_{ std::move(registrar.GetSparseSets()) },
      systems_{ std::move(registrar.GetSystems()) },
      eventManager_{ std::move(registrar.GetEventQueues()) },
      globalObjectManager_{ std::move(registrar.GetGlobalObjects()) },
      initializerSystems_{ std::move(registrar.GetInitializerSystems()) },
      environment_{ entityManager_, componentManager_, globalObjectManager_, eventManager_, dbConnection_, proxy_ }
{
    dbConnection_.prepare(Prepped_FindUserId, "SELECT * FROM player.accounts WHERE account = $1");
    dbConnection_.prepare(Prepped_CreateAccount, "INSERT INTO player.accounts (account, password) VALUES ($1, $2) RETURNING id");
    dbConnection_.prepare(Prepped_FindPlayerCharacterByName, "SELECT * FROM player.characters WHERE name = $1");
    dbConnection_.prepare(Prepped_CreatePlayerCharacter,
                          "INSERT INTO player.characters (player_id, name) VALUES ($1, $2)");
    dbConnection_.prepare(Prepped_LoadMyCharacters, "SELECT * FROM player.characters WHERE player_id = $1");
}

void MainServer::Run()
{
    for (const auto initializerSystem : initializerSystems_)
    {
        initializerSystem(environment_);
    }

    while (true)
    {
        eventManager_.Clear();

        // 종료된 세션 정리 및 이벤트 발행
        while (const auto erasedContext = networkServer_.TryClearClosedSession())
        {
            eventManager_.Push<Event_SessionErased>(*erasedContext);
        }
        networkServer_.PrepareAcceptPool();

        // 세션 수신 데이터 처리
        for (auto session : networkServer_.SessionReceiveBuffers())
        {
            while (const auto message = session.TryPeek())
            {
                stub_.HandleCts(session.Context,
                                message->MessageType,
                                message->BodySize,
                                message->Body);
                session.Pop(RatkiniaProtocol::MessageHeaderSize + message->BodySize);
            }
        }

        // 커맨드 처리
        if (commandsMutex_.try_lock())
        {
            while (!commands_.empty())
            {
                std::vector<std::string> tokens;
                std::string tokenInput;
                std::stringstream ss{ commands_.front() };

                while (ss >> tokenInput)
                {
                    tokens.emplace_back(std::move(tokenInput));
                }

                for (const auto& token : tokens)
                {
                    std::cout << token << std::endl;
                }

                if (!tokens.empty())
                {
                }

                commands_.pop();
            }

            commandsMutex_.unlock();
        }

        // 메인 게임 로직
        for (const auto system : systems_)
        {
            system(environment_);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{ 8 });
    }
}
