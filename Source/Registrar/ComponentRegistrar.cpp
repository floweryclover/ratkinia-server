//
// Created by floweryclover on 2025-08-20.
//

#include "ComponentRegistrar.h"
#include "Registrar.h"

#include "C_NameTag.h"

void RegisterComponents(Registrar& registrar)
{
    registrar.RegisterComponent<C_NameTag>();
}