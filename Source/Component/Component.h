//
// Created by floweryclover on 2025-08-20.
//

#ifndef COMPONENT_H
#define COMPONENT_H

#include "ComponentOrder.gen.h"

#define COMPONENT(Name)                                                                                                 \
private:                                                                                                                \
    inline static constexpr uint32_t RuntimeOrder = static_cast<uint32_t>(::RatkiniaProtocol::ComponentOrder::Name);    \
                                                                                                                        \
public:                                                                                                                 \
    consteval static uint32_t GetRuntimeOrder() { return RuntimeOrder; }                                                \
                                                                                                                        \
    consteval static const char* GetComponentNameStatic()                                                               \
    {                                                                                                                   \
        return #Name;                                                                                                   \
    }

#endif //COMPONENT_H
