//
// Created by floweryclover on 2025-06-20.
//

#ifndef RATKINIASERVER_ALIGNED_H
#define RATKINIASERVER_ALIGNED_H

#include <atomic>

template<typename T>
struct alignas(64) AlignedAtomic final
{
    std::atomic<T> Value;

    std::atomic<T>* operator->()
    {
        return &Value;
    }

    const std::atomic<T>* operator->() const
    {
        return &Value;
    }
};

#endif //RATKINIASERVER_ALIGNED_H
