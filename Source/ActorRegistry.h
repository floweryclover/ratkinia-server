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
    struct ChangeAssociationRequest
    {
        uint32_t Context;
        uint32_t CurrentActorId;
        uint32_t NewActorId;
    };

public:
    explicit ActorRegistry() = default;

    ~ActorRegistry() = default;

    ActorRegistry(const ActorRegistry&) = delete;

    ActorRegistry& operator=(const ActorRegistry&) = delete;

    ActorRegistry(ActorRegistry&&) = delete;

    ActorRegistry& operator=(ActorRegistry&&) = delete;

    template<typename TActor>
    void Create();

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

private:
    absl::flat_hash_map<std::string, std::unique_ptr<DynamicActor>> actors_;
    std::vector<DynamicActor*> actorRunQueue_;
};

#endif //ACTORREGISTRY_H
