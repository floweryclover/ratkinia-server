//
// Created by floweryclover on 2025-08-08.
//

#ifndef GLOBALOBJECT_H
#define GLOBALOBJECT_H

#include "ErrorMacros.h"
#include "RuntimeOrder.h"

struct GlobalObject
{
    explicit GlobalObject() = default;

    GlobalObject(const GlobalObject&) = delete;

    GlobalObject& operator=(const GlobalObject&) = delete;

    GlobalObject(GlobalObject&&) = delete;

    GlobalObject& operator=(GlobalObject&&) = delete;

    virtual ~GlobalObject() = 0;
};

inline GlobalObject::~GlobalObject()
{
}

#define GLOBALOBJECT(TClass)                                                \
    private:                                                                \
        inline static uint32_t RuntimeOrder = UnregisteredRuntimeOrder;     \
    public:                                                                 \
        static uint32_t GetRuntimeOrder()                                   \
        {                                                                   \
            return RuntimeOrder;                                            \
        }                                                                   \
                                                                            \
        static void SetRuntimeOrder(const uint32_t runtimeOrder)            \
        {                                                                   \
            CRASH_COND(RuntimeOrder != UnregisteredRuntimeOrder);           \
            RuntimeOrder = runtimeOrder;                                    \
        }                                                                   \
                                                                            \
        static const char* GetGlobalObjectName()                            \
        {                                                                   \
            return #TClass;                                                 \
        }

#endif // GLOBALOBJECT_H
