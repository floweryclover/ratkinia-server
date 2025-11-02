//
// Created by floweryclover on 2025-11-02.
//

#include "ActorNetworkInterface.h"

ActorNetworkInterface::ActorNetworkInterface(NetworkServer& networkServer)
    : networkServer_{ networkServer }
{
}

ActorNetworkInterface::~ActorNetworkInterface() = default;