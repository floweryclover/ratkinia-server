//
// Created by floweryclover on 2025-08-20.
//

#ifndef C_NAMETAG_H
#define C_NAMETAG_H

#include "Component.h"
#include <string>

struct C_NameTag final
{
    COMPONENT(NameTag)

    std::string Name;
};

#endif //C_NAMETAG_H
