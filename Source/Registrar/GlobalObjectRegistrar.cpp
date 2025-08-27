//
// Created by floweryclover on 2025-08-08.
//

#include "GlobalObjectRegistrar.h"
#include "Registrar.h"

#include "G_Auth.h"
#include "G_PlayerCharacters.h"
#include "G_Possession.h"

void RegisterGlobalObjects(Registrar& registrar)
{
    registrar.RegisterGlobalObject<G_Auth>();
    registrar.RegisterGlobalObject<G_PlayerCharacters>();
    registrar.RegisterGlobalObject<G_Possession>();
}
