//
// Created by floweryclover on 2025-08-08.
//

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

class Proxy;
class EntityManager;
class ComponentManager;
class GlobalObjectManager;
class EventManager;
class DatabaseManager;

struct ImmutableEnvironment final
{
    const EntityManager& EntityManager;
    const ComponentManager& ComponentManager;
    const GlobalObjectManager& GlobalObjectManager;
    const EventManager& EventManager;
    const DatabaseManager& DatabaseManager;
};

struct MutableEnvironment final
{
    EntityManager& EntityManager;
    ComponentManager& ComponentManager;
    GlobalObjectManager& GlobalObjectManager;
    EventManager& EventManager;
    DatabaseManager& DatabaseManager;
    Proxy& Proxy;

    ImmutableEnvironment operator()() const
    {
        return
        {
            EntityManager,
            ComponentManager,
            GlobalObjectManager,
            EventManager,
            DatabaseManager
        };
    }
};

#endif // ENVIRONMENT_H
