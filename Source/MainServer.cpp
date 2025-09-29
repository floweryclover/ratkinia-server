//
// Created by floweryclover on 2025-05-01.
//

#include "MainServer.h"
#include "NetworkServer.h"
#include "Event_SessionErased.h"

using namespace pqxx;

MainServer::MainServer(Registrar registrar,
                       const char* const listenAddress,
                       const uint16_t listenPort,
                       const char* const dbHost,
                       const uint16_t acceptPoolSize,
                       const char* const certificateFile,
                       const char* const privateKeyFile)
    : networkServer_
      {
          listenAddress,
          listenPort,
          acceptPoolSize,
          certificateFile,
          privateKeyFile
      },
      stub_{ environment_ },
      proxy_{ networkServer_ },
      database_{ dbHost },
      componentManager_{ registrar.GetSparseSets() },
      systems_{ registrar.GetSystems() },
      eventManager_{ registrar.GetEventQueues() },
      globalObjectManager_{ registrar.GetGlobalObjects() },
      initializerSystems_{ registrar.GetInitializerSystems() },
      environment_{ entityManager_, componentManager_, globalObjectManager_, eventManager_, database_, proxy_ }
{
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
