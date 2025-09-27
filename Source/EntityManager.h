//
// Created by floweryclover on 2025-08-20.
//

#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "Entity.h"
#include "ErrorMacros.h"
#include "MemoryPool/IndexedUnmanagedList.h"
#include <vector>

class EntityManager final
{
    struct VersionBlockInitializer
    {
        void operator()(const uint32_t, uint32_t& block) const
        {
            block = 0;
        }
    };

public:
    class Iterator final
    {
    public:
        explicit Iterator(
            IndexedUnmanagedList<uint32_t, VersionBlockInitializer>::ConstIterator iterator)
            : innerIterator_{ iterator }
        {
        }

        Entity operator*() const
        {
            const auto [entityId, version] = *innerIterator_;
            return Entity{ entityId, version };
        }

        bool operator==(const Iterator& rhs) const = default;

        bool operator!=(const Iterator& rhs) const = default;

        Iterator& operator++()
        {
            ++innerIterator_;
            return *this;
        }

    private:
        IndexedUnmanagedList<uint32_t, VersionBlockInitializer>::ConstIterator innerIterator_;
    };

    explicit EntityManager() = default;

    ~EntityManager() = default;

    EntityManager(const EntityManager&) = delete;

    EntityManager& operator=(const EntityManager&) = delete;

    EntityManager(EntityManager&&) = delete;

    EntityManager& operator=(EntityManager&&) = delete;

    auto begin() const
    {
        return Iterator{ list_.cbegin() };
    }

    auto end() const
    {
        return Iterator{ list_.cend() };
    }

    [[nodiscard]]
    Entity Create()
    {
        auto [index, version] = list_.Create();
        version = GetIncrementVersion(version);
        return Entity{ index, version };
    }

    [[nodiscard]]
    Entity CreateWithSpecificId(const uint32_t entityId)
    {
        ERR_FAIL_COND_V(entityId == Entity::NullId, Entity::NullEntity());

        const auto createdEntityData = list_.CreateAt(entityId);
        ERR_FAIL_NULL_V_MSG(createdEntityData, Entity::NullEntity(), entityId);

        *createdEntityData = GetIncrementVersion(*createdEntityData);

        return Entity{ entityId, *createdEntityData };
    }

    void Destroy(const Entity entity)
    {
        const auto version = list_.Get(entity.GetId());
        if (!version || *version != entity.GetVersion())
        {
            return;
        }

        list_.Erase(entity.GetId());
    }

    [[nodiscard]]
    Entity Get(const uint32_t entityId) const
    {
        const auto version = list_.Get(entityId);
        return version ? Entity{ entityId, *version } : Entity::NullEntity();
    }

private:
    IndexedUnmanagedList<uint32_t, VersionBlockInitializer> list_;

    static uint32_t GetIncrementVersion(const uint32_t version)
    {
        return (version+1) & ((0b1 << Entity::VersionBitSize) - 1);
    }
};

#endif //ENTITYMANAGER_H
