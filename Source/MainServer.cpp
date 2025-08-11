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
      ctsStub_{ environment_ },
      stcProxy_{ networkServer_ },
      dbConnection_{ DbHost },
      environment_{ globalObjectManager_, eventManager_, dbConnection_, stcProxy_ }
{
    dbConnection_.prepare(Prepped_FindUserId, "SELECT * FROM auth.accounts WHERE user_id = $1");
    dbConnection_.prepare(Prepped_InsertAccount, "INSERT INTO auth.accounts (user_id, password) VALUES ($1, $2)");
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

        for (auto buffer : networkServer_.SessionReceiveBuffers())
        {
            while (const auto message = buffer.TryPeek())
            {
                ctsStub_.HandleCts(buffer.Context,
                                   message->MessageType,
                                   message->BodySize,
                                   message->Body);
                buffer.Pop(RatkiniaProtocol::MessageHeaderSize + message->BodySize);
            }
        }

        for (const auto& system : systems_)
        {
            system(environment_);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{ 8 });
    }
}
