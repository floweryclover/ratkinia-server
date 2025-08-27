//
// Created by floweryclover on 2025-08-11.
//

#ifndef EVENT_SESSIONERASED_H
#define EVENT_SESSIONERASED_H

#include "Context.h"
#include "RuntimeOrder.h"
#include "ErrorMacros.h"

struct Event_SessionErased final
{
    uint32_t Context{ InvalidContext };

    static uint32_t GetRuntimeOrder()
    {
        return RuntimeOrder;
    }

    static void SetRuntimeOrder(const uint32_t runtimeOrder)
    {
        CRASH_COND(RuntimeOrder != UnregisteredRuntimeOrder);
        RuntimeOrder = runtimeOrder;
    }

private:
    inline static uint32_t RuntimeOrder = UnregisteredRuntimeOrder;
};

#endif //EVENT_SESSIONERASED_H
