//
// Created by floweryclover on 2025-09-30.
//

#ifndef D_COMPONENTS_H
#define D_COMPONENTS_H

#include "Database.h"

class ComponentManager;

class D_Components final : public Database
{
    DATABASE(D_Components)

public:
    void Flush(ComponentManager& componentManager);
};

#endif //D_COMPONENTS_H
