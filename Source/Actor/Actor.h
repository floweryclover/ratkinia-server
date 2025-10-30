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

class DynamicActor
{
protected:
    struct DynamicMessageHandler
    {
        const uint32_t TypeIndex;

        explicit DynamicMessageHandler(const uint32_t typeIndex)
            : TypeIndex{ typeIndex }
        {
        }

        virtual ~DynamicMessageHandler() = default;

        virtual void operator()(DynamicActor& actor, std::unique_ptr<DynamicMessage> message) = 0;
    };

public:
    explicit DynamicActor(const ActorInitializer& initializer)
        : Proxy{ initializer.Proxy.get() }, DatabaseServer{ initializer.DatabaseServer.get() }, pushIndex_{ 0 }
    {
    }

    virtual ~DynamicActor() = default;

    DynamicActor(const DynamicActor&) = delete;

    DynamicActor& operator=(const DynamicActor&) = delete;

    DynamicActor(DynamicActor&&) = delete;

    DynamicActor& operator=(DynamicActor&&) = delete;

    void HandleAllMessages();

    void PushMessage(std::unique_ptr<DynamicMessage> message)
    {
        std::scoped_lock lock{ pushMutex_ };
        messageQueue_[pushIndex_].emplace_back(std::move(message));
    }


protected:
    Proxy& Proxy;
    DatabaseServer& DatabaseServer;

    void RegisterHandler(std::unique_ptr<DynamicMessageHandler> handler);

private:
    std::mutex pushMutex_;
    int pushIndex_;
    std::vector<std::unique_ptr<DynamicMessage>> messageQueue_[2];
    absl::flat_hash_map<uint32_t, std::unique_ptr<DynamicMessageHandler>> messageHandlers_;

    virtual void OnUnknownMessageReceived(std::unique_ptr<DynamicMessage> message) = 0;
};

template<typename TDerivedActor>
class Actor : public DynamicActor
{
    template<typename TMessage>
    struct MessageHandler final : DynamicMessageHandler
    {
        explicit MessageHandler()
            : DynamicMessageHandler{ TMessage::GetTypeIndex() }
        {
        }

        void operator()(DynamicActor& actor, std::unique_ptr<DynamicMessage> message) override
        {
            static_cast<TDerivedActor&>(actor).Handle(std::unique_ptr<TMessage>(static_cast<TMessage*>(message.release())));
        }
    };

public:
    explicit Actor(const ActorInitializer& initializer)
        : DynamicActor{ initializer }
    {
    }

protected:
    template<typename TMessage>
    void Accept()
    {
        DynamicActor::RegisterHandler(std::make_unique<MessageHandler<TMessage>>());
    }

private:
    void OnUnknownMessageReceived(std::unique_ptr<DynamicMessage> message) override = 0;
};

#endif //ACTOR_H
