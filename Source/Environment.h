//
// Created by floweryclover on 2025-08-08.
//

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

class Proxy;

namespace pqxx
{
    class connection;
}

class EntityManager;
class ComponentManager;
class GlobalObjectManager;
class EventManager;

struct ImmutableEnvironment final
{
    const EntityManager& EntityManager;
    const ComponentManager& ComponentManager;
    const GlobalObjectManager& GlobalObjectManager;
    const EventManager& EventManager;
};

struct MutableEnvironment final
{
    EntityManager& EntityManager;
    ComponentManager& ComponentManager;
    GlobalObjectManager& GlobalObjectManager;
    EventManager& EventManager;
    pqxx::connection& DbConnection;
    Proxy& Proxy;

    ImmutableEnvironment operator()()
    {
        return
        {
            EntityManager,
            ComponentManager,
            GlobalObjectManager,
            EventManager
        };
    }
};

#endif // ENVIRONMENT_H
