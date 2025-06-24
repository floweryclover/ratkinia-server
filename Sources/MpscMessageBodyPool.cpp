//
// Created by floweryclover on 2025-06-13.
//

#include "MpscMessageBodyPool.h"

MpscMessageBodyPool::~MpscMessageBodyPool()
{
    for (const auto& head : memoryBlockListHeads_)
    {
        for (auto currentBlock = head.Head.load(std::memory_order_acquire); currentBlock != nullptr;)
        {
            const auto nextBlock{ currentBlock->Next };

            ::operator delete(currentBlock);

            currentBlock = nextBlock;
        }
    }
}