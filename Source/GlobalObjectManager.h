//
// Created by floweryclover on 2025-08-08.
//

#ifndef GLOBALOBJECTMANAGER_H
#define GLOBALOBJECTMANAGER_H

#include "GlobalObject.h"
#include "ErrorMacros.h"
#include <vector>
#include <memory>

class GlobalObjectManager final
{
public:
    template<typename TGlobalObject, typename ...Args>
    void Register(Args&&... args)
    {
        const uint32_t runtimeOrder = globalObjects_.size();
        TGlobalObject::SetRuntimeOrder(runtimeOrder);
        globalObjects_.emplace_back(std::make_unique<TGlobalObject>(std::forward<Args>(args)...));
    }

    template<typename TGlobalObject>
    TGlobalObject* Get()
    {
        const uint32_t runtimeOrder = TGlobalObject::GetRuntimeOrder();
        const bool isNotRegistered = runtimeOrder == (std::numeric_limits<uint32_t>::max)();
        CRASH_COND(isNotRegistered);

        return static_cast<TGlobalObject*>(globalObjects_[runtimeOrder].get());
    }

private:
    std::vector<std::unique_ptr<GlobalObject>> globalObjects_;
};

#endif //GLOBALOBJECTMANAGER_H
