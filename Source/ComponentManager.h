//
// Created by floweryclover on 2025-08-20.
//

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "Entity.h"
#include "SparseSet.h"
#include "ErrorMacros.h"
#include <memory>
#include <vector>

class ComponentManager final
{
public:
    explicit ComponentManager(std::vector<std::unique_ptr<RawSparseSet>> sparseSets)
        : sparseSets_{std::move(sparseSets)}
    {}

    ~ComponentManager() = default;

    ComponentManager(const ComponentManager&) = delete;

    ComponentManager(ComponentManager&&) = delete;

    ComponentManager& operator=(const ComponentManager&) = delete;

    ComponentManager& operator=(ComponentManager&&) = delete;

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
