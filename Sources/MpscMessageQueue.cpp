//
// Created by floweryclover on 2025-04-28.
//

#include "MpscMessageQueue.h"

void MpscMessageQueue::Enqueue(uint64_t sessionId,
                               uint16_t messageType,
                               uint16_t messageBodySize,
                               const char* messageBody)
{

    std::lock_guard lock{ mutex_ };

}

void MpscMessageQueue::Pop()
{
    auto& popQueue = queues_[1 - currentPushIndex_];
    auto& pool = pools_[1 - currentPushIndex_];

    if (popQueue.empty())
    {
        return;
    }

    pool.emplace_back(std::move(popQueue.front().MessageBody));
    popQueue.pop();
}

void MpscMessageQueue::Swap()
{
    std::lock_guard lock{ mutex_ };
    currentPushIndex_ = 1 - currentPushIndex_;
}
