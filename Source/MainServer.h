//
// Created by floweryclover on 2025-05-01.
//

#ifndef RATKINIASERVER_MAINSERVER_H
#define RATKINIASERVER_MAINSERVER_H

#include "Environment.h"
#include "NetworkServer.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"
#include "Event/EventManager.h"
#include "GlobalObject/GlobalObjectManager.h"
#include "Database/DatabaseManager.h"
#include "Stub.h"
#include "Proxy.h"
#include <absl/container/flat_hash_map.h>
#include <queue>
#include <shared_mutex>

class Actor;

class alignas(64) MainServer final
{
public:
    explicit MainServer(uint32_t mainWorkerThreadsCount,
                        const char* listenAddress,
                        uint16_t listenPort,
                        const char* dbHost,
                        uint16_t acceptPoolSize,
                        const char* certificateFile,
                        const char* privateKeyFile);

    ~MainServer();

    MainServer(const MainServer&) = delete;

    MainServer& operator=(const MainServer&) = delete;

    MainServer(MainServer&&) = delete;

    MainServer& operator=(MainServer&&) = delete;

    [[noreturn]] void Run();

    void AddCommand(std::string command)
    {
        std::lock_guard lock{ commandsMutex_ };
        commands_.emplace(std::move(command));
    }

    void PushMessage(uint32_t assosiatedActor, uint32_t context, uint16_t messageType, uint16_t bodySize, const char* body);

private:
    const uint32_t WorkerThreadsCount;
    std::queue<std::string> commands_;
    std::mutex commandsMutex_;

    NetworkServer networkServer_;
    Stub stub_;
    Proxy proxy_;

    EntityManager entityManager_;
    ComponentManager componentManager_;
    SystemManager systemManager_;
    EventManager eventManager_;
    GlobalObjectManager globalObjectManager_;
    DatabaseManager databaseManager_;

    MutableEnvironment environment_;

    std::shared_mutex actorsMutex_;
    uint32_t newActorId_;
    absl::flat_hash_map<uint32_t, std::unique_ptr<Actor>> actors_;
    std::vector<Actor*> actorRunQueue_;

    std::vector<std::thread> workerThreads_;
    uint32_t workerThreadsWorkVersion_;
    std::mutex workerThreadsWorkVersionMutex_;
    std::condition_variable workerThreadsWakeupConditionVariable_;
    bool mainThreadShouldWakeup_;
    std::mutex mainThreadShouldWakeupMutex_;
    std::condition_variable mainThreadWakeupConditionVariable_;

    alignas(64) std::atomic_uint32_t workingThreadCount_;
    alignas(64) std::atomic_uint32_t workIndex_;

    void WorkerThreadBody(uint32_t threadId);
};

#endif //RATKINIASERVER_MAINSERVER_H
