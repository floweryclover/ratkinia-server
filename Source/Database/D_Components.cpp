//
// Created by floweryclover on 2025-09-30.
//

#include "D_Components.h"
#include "ComponentManager.h"

D_Components::D_Components(pqxx::connection& connection)
    : Database{connection}
{

}

void D_Components::Flush(ComponentManager& componentManager)
{

}
