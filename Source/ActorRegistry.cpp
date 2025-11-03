//
// Created by floweryclover on 2025-11-02.
//

#include "ActorRegistry.h"
#include "A_Auth.h"

void ActorRegistry::Register(std::unique_ptr<Actor> actor)
{
    auto name = actor->Name;
    actorRunQueue_.emplace_back(actors_.emplace(std::move(actor->Name), std::move(actor)).first->second.get());
}

