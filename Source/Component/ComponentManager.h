//
// Created by floweryclover on 2025-08-20.
//

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "Manager.h"
#include "Entity.h"
#include "SparseSet.h"
#include "ErrorMacros.h"
#include <memory>
#include <vector>

class ComponentManager final : public Manager
{
public:
    template<typename TComponent>
    void RegisterComponent()
    {
        if (TComponent::GetRuntimeOrder() >= sparseSets_.size())
        {
            sparseSets_.resize(TComponent::GetRuntimeOrder() + 1);
        }

        CRASH_COND_MSG(sparseSets_[TComponent::GetRuntimeOrder()] != nullptr, TComponent::GetComponentNameStatic());
        sparseSets_[TComponent::GetRuntimeOrder()] = std::make_unique<SparseSet<TComponent>>();
    }

    template<typename TComponent>
    TComponent* AttachComponentTo(const Entity entity)
    {
        auto& sparseSet = static_cast<SparseSet<TComponent>&>(*sparseSets_[TComponent::GetRuntimeOrder()]);
        auto& emplaced = sparseSet.Data.emplace_back();
        emplaced.first = entity;
        return &emplaced.second;
    }

    template<typename TComponent>
    std::vector<std::pair<Entity, TComponent>>& Components()
    {
        auto& sparseSet = static_cast<SparseSet<TComponent>&>(*sparseSets_[TComponent::GetRuntimeOrder()]);
        return sparseSet.Data;
    }

    template<typename TComponent>
    TComponent* GetComponentOf(const Entity entity)
    {
        auto& sparseSet = static_cast<SparseSet<TComponent>&>(*sparseSets_[TComponent::GetRuntimeOrder()]);
        for (auto& [componentOwnerEntity, component] : sparseSet.Data)
        {
            if (entity == componentOwnerEntity)
            {
                return &component;
            }
        }
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<RawSparseSet>> sparseSets_;
};

#endif //COMPONENTMANAGER_H
