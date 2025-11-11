//
// Created by floweryclover on 2025-10-22.
//

#ifndef ACTOR_H
#define ACTOR_H

#include "Message.h"
#include "ErrorMacros.h"
#include <absl/container/flat_hash_map.h>
#include <vector>
#include <mutex>

class ActorNetworkInterface;
class ActorMessageDispatcher;
class DbConnectionPool;

struct ActorInitializer final
{
    std::reference_wrapper<ActorNetworkInterface> ActorNetworkInterface;
    std::reference_wrapper<ActorMessageDispatcher> ActorMessageDispatcher;
    std::reference_wrapper<DbConnectionPool> DbConnectionPool;
};

template <typename... TMessage>
struct Accept
{
};

class DynamicActor
{
public:
    const std::string Name;

    explicit DynamicActor(std::string name, const ActorInitializer& initializer)
        : Name{std::move(name)},
          Network{initializer.ActorNetworkInterface.get()},
          Dispatcher{initializer.ActorMessageDispatcher.get()},
          DbConnectionPool{initializer.DbConnectionPool.get()},
          pushIndex_{0}
    {
    }

    virtual ~DynamicActor() = default;

    DynamicActor(const DynamicActor&) = delete;

    DynamicActor& operator=(const DynamicActor&) = delete;

    DynamicActor(DynamicActor&&) = delete;

    DynamicActor& operator=(DynamicActor&&) = delete;

    void Run();

    void PushMessage(std::unique_ptr<DynamicMessage> message)
    {
        ERR_FAIL_COND(message == nullptr);

        std::scoped_lock lock{pushMutex_};
        messageQueue_[pushIndex_].emplace_back(std::move(message));
    }

protected:
    ActorNetworkInterface& Network;
    ActorMessageDispatcher& Dispatcher;
    DbConnectionPool& DbConnectionPool;

    virtual void Tick()
    {
    }

    void RegisterHandler(const uint32_t typeIndex, void (*handler)(DynamicActor&, std::unique_ptr<DynamicMessage>))
    {
        const auto [iter, emplaced] = messageHandlers_.try_emplace(typeIndex, handler);
        CRASH_COND(!emplaced);
    }

private:
    std::mutex pushMutex_;
    int pushIndex_;
    std::vector<std::unique_ptr<DynamicMessage>> messageQueue_[2];
    absl::flat_hash_map<uint32_t, void(*)(DynamicActor&, std::unique_ptr<DynamicMessage>)> messageHandlers_;

    virtual void OnUnknownMessageReceived(std::unique_ptr<DynamicMessage> message) = 0;
};

template <typename TDerivedActor>
class Actor : public DynamicActor
{
public:
    explicit Actor(std::string name, const ActorInitializer& initializer)
        : DynamicActor{std::move(name), initializer}
    {
        RegisterHandlers(static_cast<TDerivedActor*>(nullptr));
    }

private:
    template <typename... TMessages>
    void RegisterHandlers(Accept<TMessages...>*)
    {
        (RegisterHandler(TMessages::GetTypeIndex(), [](DynamicActor& actor, std::unique_ptr<DynamicMessage> message)
        {
            static_cast<TDerivedActor&>(actor).Handle(
                std::unique_ptr<TMessages>(static_cast<TMessages*>(message.release())));
        }), ...);
    }

    void RegisterHandlers(...)
    {
    }
};

#define ACTOR(TActor)                                                                               \
public:                                                                                             \
    explicit TActor(const ActorInitializer& initializer) : Actor(#TActor, initializer) {} \
private:

#endif //ACTOR_H
