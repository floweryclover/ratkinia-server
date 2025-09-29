//
// Created by floweryclover on 2025-08-08.
//

#ifndef GLOBALOBJECTMANAGER_H
#define GLOBALOBJECTMANAGER_H

#include "Manager.h"
#include "GlobalObject.h"
#include "ErrorMacros.h"
#include <vector>
#include <memory>

class GlobalObjectManager final : public Manager
{
public:
    template<typename TGlobalObject>
    void Register()
    {
        TGlobalObject::SetRuntimeOrder(globalObjects_.size());
        globalObjects_.emplace_back(std::make_unique<TGlobalObject>());
    }

    template<typename TGlobalObject>
    TGlobalObject& Get()
    {
        return *static_cast<TGlobalObject*>(globalObjects_[TGlobalObject::GetRuntimeOrder()].get());
    }

private:
    std::vector<std::unique_ptr<GlobalObject>> globalObjects_;
};

#endif //GLOBALOBJECTMANAGER_H
