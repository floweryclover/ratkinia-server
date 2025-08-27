//
// Created by floweryclover on 2025-08-20.
//

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

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
    TComponent* AttachComponentTo(const uint32_t entity)
    {
        auto& sparseSet = static_cast<SparseSet<TComponent>&>(*sparseSets_[TComponent::GetRuntimeOrder()]);
        return &sparseSet.Data.emplace_back(entity).second;
    }

    template<typename TComponent>
    std::vector<std::pair<uint32_t, TComponent>>& Components()
    {
        auto& sparseSet = static_cast<SparseSet<TComponent>&>(*sparseSets_[TComponent::GetRuntimeOrder()]);
        return sparseSet.Data;
    }

    template<typename TComponent>
    TComponent* GetComponentOf(const uint32_t entity)
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
