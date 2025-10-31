//
// Created by floweryclover on 2025-10-22.
//

#ifndef ACTOR_H
#define ACTOR_H

#include "Message.h"
#include <absl/container/flat_hash_map.h>
#include <vector>
#include <mutex>

class Proxy;
class DatabaseServer;

struct ActorInitializer final
{
    std::reference_wrapper<Proxy> Proxy;
    std::reference_wrapper<DatabaseServer> DatabaseServer;
};

class Actor
{
public:
    explicit Actor(const ActorInitializer& initializer)
        : Proxy{ initializer.Proxy.get() }, DatabaseServer{ initializer.DatabaseServer.get() }, pushIndex_{ 0 }
    {
    }

    virtual ~Actor() = default;

    Actor(const Actor&) = delete;

    Actor& operator=(const Actor&) = delete;

    Actor(Actor&&) = delete;

    Actor& operator=(Actor&&) = delete;

    void HandleAllMessages();

    void PushMessage(std::unique_ptr<DynamicMessage> message)
    {
        std::scoped_lock lock{ pushMutex_ };
        messageQueue_[pushIndex_].emplace_back(std::move(message));
    }

protected:
    Proxy& Proxy;
    DatabaseServer& DatabaseServer;

    template<typename TMessage, typename TDerivedActor>
    void Accept(TDerivedActor*)
    {
        const auto [iter, emplaced] = messageHandlers_.emplace(
            TMessage::GetTypeIndex(),
            [](Actor& actor, std::unique_ptr<DynamicMessage> message)
            {
                static_cast<TDerivedActor&>(actor).Handle(std::unique_ptr<TMessage>(static_cast<TMessage*>(message.release())));
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
