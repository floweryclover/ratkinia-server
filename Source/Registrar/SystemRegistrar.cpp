//
// Created by floweryclover on 2025-08-08.
//

#include "SystemRegistrar.h"
#include "Registrar.h"

#include "S_Auth.h"
#include "S_PlayerCharacters_UnpossessOnDisconnected.h"

#include "Initializer/S_Initializer_PlayerCharacters.h"

void RegisterInitializerSystems(Registrar& registrar);

void RegisterSystems(Registrar& registrar)
{
    RegisterInitializerSystems(registrar);

    registrar.RegisterSystem(&S_Auth);
    registrar.RegisterSystem(&S_PlayerCharacters_UnpossessOnDisconnected);
}

void RegisterInitializerSystems(Registrar& registrar)
{
    registrar.RegisterInitializerSystem(&S_Initializer_PlayerCharacters);
}

