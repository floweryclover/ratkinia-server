//
// Created by floweryclover on 2025-05-01.
//

#ifndef RATKINIASERVER_MAINSERVER_H
#define RATKINIASERVER_MAINSERVER_H

#include "Environment.h"
#include "Registrar.h"
#include "EntityManager.h"
#include "NetworkServer.h"
#include "ComponentManager.h"
#include "DatabaseManager.h"
#include "GlobalObjectManager.h"
#include "EventManager.h"
#include "System.h"
#include "Stub.h"
#include "Proxy.h"
#include "ErrorMacros.h"
#include <thread>
#include <queue>

class NetworkServer;

class MainServer final
{
public:
    explicit MainServer(Registrar registrar,
                        const char* listenAddress,
                        uint16_t listenPort,
                        const char* dbHost,
                        uint16_t acceptPoolSize,
                        const char* certificateFile,
                        const char* privateKeyFile);

    ~MainServer() = default;

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
    std::queue<std::string> commands_;
    std::mutex commandsMutex_;

    NetworkServer networkServer_;
    Stub stub_;
    Proxy proxy_;
    DatabaseManager database_;
    std::thread thread_;

    EntityManager entityManager_;
    ComponentManager componentManager_;
    std::vector<System> systems_;
    EventManager eventManager_;
    GlobalObjectManager globalObjectManager_;

    std::vector<System> initializerSystems_;

    MutableEnvironment environment_;
};

#endif //RATKINIASERVER_MAINSERVER_H
