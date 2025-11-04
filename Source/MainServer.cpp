//
// Created by floweryclover on 2025-05-01.
//

#include "MainServer.h"
#include "ActorMessageDispatcher.h"
#include "DbConnectionPool.h"
#include "NetworkServer.h"
#include "ActorNetworkInterface.h"
#include "ActorRegistry.h"
#include "A_Auth.h"

MainServer::MainServer(const uint32_t mainWorkerThreadsCount,
                       const char* const listenAddress,
                       const uint16_t listenPort,
                       const char* const dbHost,
                       const size_t maxDbConnections,
                       const uint16_t acceptPoolSize,
                       const char* const certificateFile,
                       const char* const privateKeyFile)
    : WorkerThreadsCount{ mainWorkerThreadsCount },
      ActorRegistry{ std::make_unique<class ActorRegistry>() },
      ActorMessageDispatcher{ std::make_unique<class ActorMessageDispatcher>(*ActorRegistry) },
      NetworkServer
      {
          std::make_unique<class NetworkServer>(
              "A_Auth",
              *ActorMessageDispatcher,
              listenAddress,
              listenPort,
              acceptPoolSize,
              certificateFile,
              privateKeyFile)
      },
      ActorNetworkInterface{ std::make_unique<class ActorNetworkInterface>(*NetworkServer) },
      DbConnectionPool{ std::make_unique<class DbConnectionPool>(dbHost, maxDbConnections) },
      workerThreadsWorkVersion_{ 0 },
      mainThreadShouldWakeup_{ false },
      workingThreadCount_{ WorkerThreadsCount }
{
    for (int i = 1; i <= WorkerThreadsCount; ++i)
    {
        workerThreads_.emplace_back(&MainServer::WorkerThreadBody, this, i);
    }

    ActorRegistry->Register(std::make_unique<A_Auth>(
        ActorInitializer
        {
            "A_Auth",
            *ActorNetworkInterface,
            *ActorMessageDispatcher,
            *DbConnectionPool
        }));
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
            const auto actor = ActorRegistry->Get(workIndex);
            if (!actor)
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

            actor->Run();
        }
    }
    // ReSharper disable once CppDFAUnreachableCode
}
