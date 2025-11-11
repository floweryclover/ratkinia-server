//
// Created by floweryclover on 2025-11-02.
//

#ifndef ACTORREGISTRY_H
#define ACTORREGISTRY_H

#include "Actor.h"
#include "ActorFacade.h"
#include <absl/container/flat_hash_map.h>
#include <memory>

class ActorFacade;

class ActorRegistry final
{
public:
    explicit ActorRegistry();

    ~ActorRegistry();

    ActorRegistry(const ActorRegistry&) = delete;

    ActorRegistry& operator=(const ActorRegistry&) = delete;

    ActorRegistry(ActorRegistry&&) = delete;

    ActorRegistry& operator=(ActorRegistry&&) = delete;

    void Register(std::unique_ptr<DynamicActor> actor);

    [[nodiscard]]
    bool TryPushMessageTo(const auto& actorName, std::unique_ptr<DynamicMessage> message)
    {
        const auto iter = actors_.find(actorName);
        if (iter == actors_.end())
        {
            return false;
        }

        iter->second->PushMessage(std::move(message));
        return true;
    }

    template<typename TFacade>
    void CreateFacade()
    {
        const auto [iter, emplaced] = actorFacades_.try_emplace(TFacade::GetRuntimeIndex(), std::make_unique<TFacade>());
        CRASH_COND(!emplaced);
    }

    template<typename TFacade>
    TFacade& GetFacade()
    {
        const auto iter = actorFacades_.find(TFacade::GetRuntimeIndex());
        CRASH_COND(iter == actorFacades_.end());
        return *static_cast<TFacade*>(iter->second.get());
    }

    [[nodiscard]]
    bool RunByIndex(const size_t index)
    {
        if (index >= actorRunQueue_.size())
        {
            return false;
        }

        actorRunQueue_[index]->Run();
        return true;
    }

private:
    uint32_t newActorIndex_;
    absl::flat_hash_map<uint32_t, std::unique_ptr<DynamicActor>> actors_;
    std::vector<DynamicActor*> actorRunQueue_;

    absl::flat_hash_map<uint32_t, std::unique_ptr<ActorFacade>> actorFacades_;
};

#endif //ACTORREGISTRY_H
