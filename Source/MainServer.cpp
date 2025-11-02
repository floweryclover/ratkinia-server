//
// Created by floweryclover on 2025-05-01.
//

#include "MainServer.h"
#include "A_Auth.h"
#include "DatabaseRegistrar.h"
#include "NetworkServer.h"
#include "DatabaseServer.h"
#include "ActorNetworkInterface.h"
#include "Msg_Cts.h"

MainServer::MainServer(const uint32_t mainWorkerThreadsCount,
                       const char* const listenAddress,
                       const uint16_t listenPort,
                       const char* const dbHost,
                       const uint16_t acceptPoolSize,
                       const char* const certificateFile,
                       const char* const privateKeyFile)
    : WorkerThreadsCount{ mainWorkerThreadsCount },
      NetworkServer
      {
          std::make_unique<class NetworkServer>(
              0,
              [this](const uint32_t assosiatedActor,
                     const uint32_t context,
                     const uint16_t messageType,
                     const uint16_t bodySize,
                     const char* const body)
              {
                  PushMessage(assosiatedActor, context, messageType, bodySize, body);
              },
              listenAddress,
              listenPort,
              acceptPoolSize,
              certificateFile,
              privateKeyFile)
      },
      DatabaseServer{ std::make_unique<class DatabaseServer>(dbHost) },
      ActorNetworkInterface{ std::make_unique<class ActorNetworkInterface>(*NetworkServer) },
      newActorId_{ 1 },
      workerThreadsWorkVersion_{ 0 },
      mainThreadShouldWakeup_{ false },
      workingThreadCount_{ WorkerThreadsCount }
{
    actorRunQueue_.emplace_back(actors_.emplace(0, std::make_unique<A_Auth>(ActorInitializer{*ActorNetworkInterface, *DatabaseServer})).first->second.get());

    for (int i = 1; i <= WorkerThreadsCount; ++i)
    {
        workerThreads_.emplace_back(&MainServer::WorkerThreadBody, this, i);
    }

    RegisterDatabase(*DatabaseServer);
    DatabaseServer->Finalize();
}

MainServer::~MainServer() = default;

void MainServer::Run()
{
    while (true)
    {
        workingThreadCount_.store(WorkerThreadsCount, std::memory_order_relaxed);
        workIndex_.store(0, std::memory_order_relaxed);
        {
            std::scoped_lock lock{ workerThreadsWorkVersionMutex_ };
            ++workerThreadsWorkVersion_;
        }
        workerThreadsWakeupConditionVariable_.notify_all();

        {
            std::unique_lock lock{ mainThreadShouldWakeupMutex_ };
            mainThreadWakeupConditionVariable_.wait(
                lock,
                [this]
                {
                    return mainThreadShouldWakeup_;
                });
            mainThreadShouldWakeup_ = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{ 16 });
    }

    // ReSharper disable once CppDFAUnreachableCode
}

void MainServer::PushMessage(const uint32_t assosiatedActor,
                             const uint32_t context,
                             const uint16_t messageType,
                             const uint16_t bodySize,
                             const char* const body)
{
    auto ownedBody = std::make_unique<char[]>(bodySize);
    memcpy(ownedBody.get(), body, bodySize);

    auto message = std::make_unique<Msg_Cts>();
    message->Context = context;
    message->MessageType = messageType;
    message->BodySize = bodySize;
    message->Body = std::move(ownedBody);

    std::shared_lock lock{ actorsMutex_ };
    CRASH_COND_MSG(!actors_.contains(assosiatedActor), assosiatedActor);
    actors_.at(assosiatedActor)->PushMessage(std::move(message));
}

void MainServer::WorkerThreadBody(const uint32_t)
{
    uint32_t desiredVersion = 1;
    while (true)
    {
        {
            std::unique_lock lock{ workerThreadsWorkVersionMutex_ };
            workerThreadsWakeupConditionVariable_.wait(lock,
                                                       [&]
                                                       {
                                                           return workerThreadsWorkVersion_ == desiredVersion;
                                                       });
        }

        while (true)
        {
            const uint32_t workIndex = workIndex_.fetch_add(1, std::memory_order_relaxed);
            if (workIndex >= actorRunQueue_.size())
            {
                if (workingThreadCount_.fetch_sub(1, std::memory_order_relaxed) == 1)
                {
                    std::scoped_lock lock{ mainThreadShouldWakeupMutex_ };
                    mainThreadShouldWakeup_ = true;
                    mainThreadWakeupConditionVariable_.notify_one();
                }

                ++desiredVersion;
                break;
            }

            actorRunQueue_[workIndex]->HandleAllMessages();
        }
    }
    // ReSharper disable once CppDFAUnreachableCode
}
