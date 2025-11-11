//
// Created by floweryclover on 2025-11-02.
//

#include "ActorRegistry.h"

ActorRegistry::ActorRegistry()
    : newActorIndex_{0}
{
}

ActorRegistry::~ActorRegistry() = default;

void ActorRegistry::Register(std::unique_ptr<DynamicActor> actor)
{
    actorRunQueue_.emplace_back(actors_.emplace(newActorIndex_, std::move(actor)).first->second.get());
    ++newActorIndex_;
}
