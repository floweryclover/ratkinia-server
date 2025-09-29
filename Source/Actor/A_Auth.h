//
// Created by floweryclover on 2025-10-25.
//

#ifndef A_AUTH_H
#define A_AUTH_H

#include "Actor.h"

class A_Auth final : public Actor
{
protected:
    void OnHandleMessage(uint32_t context, uint16_t messageType, uint16_t bodySize, const char* body) override
    {

    }
};

#endif //A_AUTH_H
