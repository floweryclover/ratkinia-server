//
// Created by floweryclover on 2025-05-01.
//

#include "MainServer.h"
#include "NetworkServer.h"
#include "Database.h"
#include <pqxx/pqxx>

using namespace Database;
using namespace pqxx;

MainServer::MainServer(std::string listenAddress,
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
      environment_{ globalObjectManager_, eventManager_, dbConnection_, proxy_ }
{
    dbConnection_.prepare(Prepped_FindUserId, "SELECT * FROM player.accounts WHERE account = $1");
    dbConnection_.prepare(Prepped_CreateAccount, "INSERT INTO player.accounts (account, password) VALUES ($1, $2)");
    dbConnection_.prepare(Prepped_FindPlayerCharacterByName, "SELECT * FROM player.characters WHERE name = $1");
    dbConnection_.prepare(Prepped_CreatePlayerCharacter, "INSERT INTO player.characters (player_id, name) VALUES ($1, $2)");
}

void MainServer::Run()
{
    while (true)
    {
        eventManager_.Clear();
        while (const auto erasedContext = networkServer_.TryClearClosedSession())
        {
            eventManager_.Push<Event_SessionErased>(*erasedContext);
        }
        networkServer_.PrepareAcceptPool();

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

        for (const auto& system : systems_)
        {
            system(environment_);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{ 8 });
    }
}
