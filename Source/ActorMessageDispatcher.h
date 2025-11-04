//
// Created by floweryclover on 2025-11-02.
//

#ifndef READONLYACTORREGISTRY_H
#define READONLYACTORREGISTRY_H

#include "ActorRegistry.h"

class ActorMessageDispatcher final
{
public:
    explicit ActorMessageDispatcher(ActorRegistry& actorRegistry)
        : actorRegistry_{ actorRegistry }
    {
    }

    ~ActorMessageDispatcher() = default;

    ActorMessageDispatcher(const ActorMessageDispatcher&) = delete;

    ActorMessageDispatcher& operator=(const ActorMessageDispatcher&) = delete;

    ActorMessageDispatcher(ActorMessageDispatcher&&) = delete;

    ActorMessageDispatcher& operator=(ActorMessageDispatcher&&) = delete;

    [[nodiscard]]
    // ReSharper disable once CppMemberFunctionMayBeConst
    bool TryPushMessageTo(const auto& actorName, std::unique_ptr<DynamicMessage> message)
    {
        return actorRegistry_.TryPushMessageTo(actorName, std::move(message));
    }

private:
    ActorRegistry& actorRegistry_;
};

#endif //READONLYACTORREGISTRY_H
