//
// Created by floweryclover on 2025-11-02.
//

#ifndef ACTORREGISTRY_H
#define ACTORREGISTRY_H

#include "Actor.h"
#include <absl/container/flat_hash_map.h>
#include <memory>

class ActorRegistry final
{
public:
    explicit ActorRegistry() = default;

    ~ActorRegistry() = default;

    ActorRegistry(const ActorRegistry&) = delete;

    ActorRegistry& operator=(const ActorRegistry&) = delete;

    ActorRegistry(ActorRegistry&&) = delete;

    ActorRegistry& operator=(ActorRegistry&&) = delete;

    void Register(std::unique_ptr<Actor> actor);

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

    [[nodiscard]]
    Actor* Get(const uint32_t actorIndex)
    {
        return actorIndex < actorRunQueue_.size() ? actorRunQueue_[actorIndex] : nullptr;
    }

private:
    absl::flat_hash_map<std::string, std::unique_ptr<Actor>> actors_;
    std::vector<Actor*> actorRunQueue_;
};

#endif //ACTORREGISTRY_H
