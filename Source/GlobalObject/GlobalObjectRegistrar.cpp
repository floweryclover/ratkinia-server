//
// Created by floweryclover on 2025-08-08.
//

#include "GlobalObjectRegistrar.h"
#include "GlobalObjectManager.h"

#include "G_PlayerCharacters.h"
#include "G_Possession.h"

void RegisterGlobalObjects(GlobalObjectManager& globalObjectManager)
{
    globalObjectManager.Register<G_PlayerCharacters>();
    globalObjectManager.Register<G_Possession>();
}
