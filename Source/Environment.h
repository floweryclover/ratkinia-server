//
// Created by floweryclover on 2025-08-08.
//

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

class StcProxy;

namespace pqxx
{
    class connection;
}

class GlobalObjectManager;
class EventManager;

struct ImmutableEnvironment final
{
    const GlobalObjectManager& GlobalObjectManager;
    const EventManager& EventManager;
};

struct MutableEnvironment final
{
    GlobalObjectManager& GlobalObjectManager;
    EventManager& EventManager;
    pqxx::connection& DbConnection;
    StcProxy& StcProxy;

    ImmutableEnvironment operator()()
    {
        return
        {
            GlobalObjectManager,
            EventManager
        };
    }
};

#endif // ENVIRONMENT_H
