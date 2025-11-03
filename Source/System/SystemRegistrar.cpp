//
// Created by floweryclover on 2025-08-08.
//

#include "SystemManager.h"
#include "S_PlayerCharacters_UnpossessOnDisconnected.h"
#include "Initializer/S_Initializer_PlayerCharacters.h"

void RegisterInitializerSystems(SystemManager& systemManager);

void RegisterSystems(SystemManager& systemManager)
{
    RegisterInitializerSystems(systemManager);

    systemManager.RegisterSystem<S_PlayerCharacters_UnpossessOnDisconnected>();
}

void RegisterInitializerSystems(SystemManager& systemManager)
{
    systemManager.RegisterInitializerSystem<S_Initializer_PlayerCharacters>();
}

