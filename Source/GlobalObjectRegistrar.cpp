//
// Created by floweryclover on 2025-08-08.
//

#include "GlobalObjectRegistrar.h"
#include "MainServer.h"
#include "G_Auth.h"

void RegisterGlobalObjects(MainServer& gameServer)
{
    gameServer.RegisterGlobalObject<G_Auth>();
}
