//
// Created by floweryclover on 2025-10-30.
//

#ifndef MESSAGE_H
#define MESSAGE_H

#include <atomic>
#include <cstdint>

struct DynamicMessage
{
    explicit DynamicMessage(const uint32_t typeIndex)
        : TypeIndex{ typeIndex }
    {
    }

    const uint32_t TypeIndex;

protected:
    static uint32_t GetNextTypeIndex()
    {
        static std::atomic_uint32_t nextTypeIndex;
        return nextTypeIndex.fetch_add(1, std::memory_order_relaxed);
    }
};

template<typename>
struct Message : DynamicMessage
{
    explicit Message()
        : DynamicMessage{ GetTypeIndex() }
    {
    }

    static uint32_t GetTypeIndex()
    {
        static uint32_t index = GetNextTypeIndex();
        return index;
    }
};

#endif //MESSAGE_H
