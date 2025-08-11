//
// Created by floweryclover on 2025-08-11.
//

#ifndef EVENT_SESSIONERASED_H
#define EVENT_SESSIONERASED_H

#include "Context.h"

struct Event_SessionErased final
{
    uint32_t Context{ InvalidContext };

    static uint32_t GetRuntimeOrder()
    {
        return 0;
    }
};

#endif //EVENT_SESSIONERASED_H
