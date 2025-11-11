//
// Created by floweryclover on 2025-11-02.
//

#ifndef ACTORMESSAGEDISPATCHER_H
#define ACTORMESSAGEDISPATCHER_H

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

#endif //ACTORMESSAGEDISPATCHER_H
