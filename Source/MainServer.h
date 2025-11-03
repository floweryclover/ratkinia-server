//
// Created by floweryclover on 2025-05-01.
//

#ifndef RATKINIASERVER_MAINSERVER_H
#define RATKINIASERVER_MAINSERVER_H

#include <absl/container/flat_hash_map.h>
#include <queue>
#include <shared_mutex>

class ActorMessageDispatcher;
class ActorRegistry;
class ActorNetworkInterface;
class Actor;
class NetworkServer;
class DatabaseServer;
class Proxy;

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

private:
    const uint32_t WorkerThreadsCount;
    std::queue<std::string> commands_;
    std::mutex commandsMutex_;

    const std::unique_ptr<ActorRegistry> ActorRegistry;
    const std::unique_ptr<ActorMessageDispatcher> ActorMessageDispatcher;
    const std::unique_ptr<NetworkServer> NetworkServer;
    const std::unique_ptr<ActorNetworkInterface> ActorNetworkInterface;
    const std::unique_ptr<DatabaseServer> DatabaseServer;

    std::vector<std::thread> workerThreads_;
    uint32_t workerThreadsWorkVersion_;
    std::mutex workerThreadsWorkVersionMutex_;
    std::condition_variable workerThreadsWakeupConditionVariable_;
    bool mainThreadShouldWakeup_;
    std::mutex mainThreadShouldWakeupMutex_;
    std::condition_variable mainThreadWakeupConditionVariable_;

    alignas(64) std::atomic_uint32_t workingThreadCount_;
    alignas(64) std::atomic_uint32_t workIndex_;

    [[noreturn]] void WorkerThreadBody(uint32_t threadId);
};

#endif //RATKINIASERVER_MAINSERVER_H
