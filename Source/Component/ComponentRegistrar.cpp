//
// Created by floweryclover on 2025-08-20.
//

#include "ComponentRegistrar.h"
#include "ComponentManager.h"

#include "C_HumanLikeBody.h"
#include "C_NameTag.h"

void RegisterComponents(ComponentManager& componentManager)
{
    componentManager.RegisterComponent<C_HumanLikeBody>();
    componentManager.RegisterComponent<C_NameTag>();
}