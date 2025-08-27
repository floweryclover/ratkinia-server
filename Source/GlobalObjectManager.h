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
    explicit GlobalObjectManager(std::vector<std::unique_ptr<GlobalObject>> globalObjects)
        : globalObjects_{ std::move(globalObjects) }
    {
    }

    ~GlobalObjectManager() = default;

    GlobalObjectManager(const GlobalObjectManager&) = delete;

    GlobalObjectManager(GlobalObjectManager&&) = delete;

    GlobalObjectManager& operator=(const GlobalObjectManager&) = delete;

    GlobalObjectManager& operator=(GlobalObjectManager&&) = delete;

    template<typename TGlobalObject>
    TGlobalObject& Get()
    {
        const uint32_t runtimeOrder = TGlobalObject::GetRuntimeOrder();
        const bool isNotRegistered = runtimeOrder == (std::numeric_limits<uint32_t>::max)();
        CRASH_COND_MSG(isNotRegistered, TGlobalObject::GetGlobalObjectName());

        return *static_cast<TGlobalObject*>(globalObjects_[runtimeOrder].get());
    }

private:
    std::vector<std::unique_ptr<GlobalObject>> globalObjects_;
};

#endif //GLOBALOBJECTMANAGER_H
