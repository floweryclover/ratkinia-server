//
// Created by floweryclover on 2025-08-08.
//

#include "SystemRegistrar.h"
#include "S_Auth.h"
#include "MainServer.h"

void RegisterSystems(MainServer& gameServer)
{
    gameServer.RegisterSystem(&S_Auth, "S_Auth");
}
