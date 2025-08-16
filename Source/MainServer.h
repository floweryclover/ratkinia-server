//
// Created by floweryclover on 2025-05-01.
//

#ifndef RATKINIASERVER_MAINSERVER_H
#define RATKINIASERVER_MAINSERVER_H

#include "Environment.h"
#include "NetworkServer.h"
#include "GlobalObjectManager.h"
#include "EventManager.h"
#include "Floweryclover/ParallelExecutor.h"
#include "System.h"
#include "Stub.h"
#include "Proxy.h"
#include "ErrorMacros.h"
#include <pqxx/pqxx>
#include <thread>
#include <unordered_set>

class NetworkServer;

class MainServer final
{
public:
    explicit MainServer(std::string listenAddress,
                        uint16_t listenPort,
                        std::string dbHost,
                        uint16_t acceptPoolSize,
                        const char* certificateFile,
                        const char* privateKeyFile);

    ~MainServer() = default;

    MainServer(const MainServer&) = delete;

    MainServer& operator=(const MainServer&) = delete;

    MainServer(MainServer&&) = delete;

    MainServer& operator=(MainServer&&) = delete;

    [[noreturn]] void Run();

    [[nodiscard]]
    pqxx::connection& GetDbConnection()
    {
        CRASH_COND(!dbConnection_.is_open());
        return dbConnection_;
    }

    template<typename TGlobalObject, typename... Args>
    void RegisterGlobalObject(Args&&... args)
    {
        globalObjectManager_.Register<TGlobalObject>(std::forward<Args>(args)...);
    }

    void RegisterSystem(const System system, std::string name)
    {
        ERR_FAIL_COND(registeredSystems_.contains(name));
        systems_.emplace_back(system);
        registeredSystems_.emplace(std::move(name));
    }

private:
    const std::string ListenAddress;
    const uint16_t ListenPort;
    const std::string DbHost;

    NetworkServer networkServer_;
    Floweryclover::ParallelExecutor executor_;
    Stub stub_;
    Proxy proxy_;
    pqxx::connection dbConnection_;
    std::thread thread_;

    GlobalObjectManager globalObjectManager_;
    EventManager eventManager_;

    std::vector<System> systems_;
    std::unordered_set<std::string> registeredSystems_;

    MutableEnvironment environment_;
};


#endif //RATKINIASERVER_MAINSERVER_H
