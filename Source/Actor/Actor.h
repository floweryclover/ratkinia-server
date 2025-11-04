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
    const char* const Name;
    std::reference_wrapper<ActorNetworkInterface> ActorNetworkInterface;
    std::reference_wrapper<ActorMessageDispatcher> ActorMessageDispatcher;
    std::reference_wrapper<DbConnectionPool> DbConnectionPool;
};

class Actor
{
public:
    const std::string Name;

    explicit Actor(const ActorInitializer& initializer)
        : Name{ initializer.Name },
          Network{ initializer.ActorNetworkInterface.get() },
          Dispatcher{ initializer.ActorMessageDispatcher.get() },
          DbConnectionPool{ initializer.DbConnectionPool.get() },
          pushIndex_{ 0 }
    {
    }

    virtual ~Actor() = default;

    Actor(const Actor&) = delete;

    Actor& operator=(const Actor&) = delete;

    Actor(Actor&&) = delete;

    Actor& operator=(Actor&&) = delete;

    void Run();

    void PushMessage(std::unique_ptr<DynamicMessage> message)
    {
        ERR_FAIL_COND(message == nullptr);

        std::scoped_lock lock{ pushMutex_ };
        messageQueue_[pushIndex_].emplace_back(std::move(message));
    }

protected:
    ActorNetworkInterface& Network;
    ActorMessageDispatcher& Dispatcher;
    DbConnectionPool& DbConnectionPool;

    virtual void Tick()
    {
    }

    template<typename TMessage>
    void Accept([[maybe_unused]] auto derivedActor)
    {
        const auto [iter, emplaced] = messageHandlers_.emplace(
            TMessage::GetTypeIndex(),
            [](Actor& actor, std::unique_ptr<DynamicMessage> message)
            {
                static_cast<std::remove_pointer_t<decltype(derivedActor)>&>(actor).Handle(
                    std::unique_ptr<TMessage>(static_cast<TMessage*>(message.release())));
            });
    }

private:
    std::mutex pushMutex_;
    int pushIndex_;
    std::vector<std::unique_ptr<DynamicMessage>> messageQueue_[2];
    absl::flat_hash_map<uint32_t, void(*)(Actor&, std::unique_ptr<DynamicMessage>)> messageHandlers_;

    virtual void OnUnknownMessageReceived(std::unique_ptr<DynamicMessage> message) = 0;
};

#endif //ACTOR_H
