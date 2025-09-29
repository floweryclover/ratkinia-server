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
#include "ErrorMacros.h"
#include <queue>

class MainServer final
{
public:
    explicit MainServer(const char* listenAddress,
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

    EntityManager entityManager_;
    ComponentManager componentManager_;
    SystemManager systemManager_;
    EventManager eventManager_;
    GlobalObjectManager globalObjectManager_;
    DatabaseManager databaseManager_;

    MutableEnvironment environment_;
};

#endif //RATKINIASERVER_MAINSERVER_H
