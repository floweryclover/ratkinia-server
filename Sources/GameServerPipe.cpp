//
// Created by floweryclover on 2025-04-28.
//

#include "GameServerPipe.h"
#include "MessagePrinter.h"
#include "Errors.h"

GameServerPipe::GameServerPipe()
    : currentPushIndex_{ 0 }
{

}

bool GameServerPipe::Push(PipeMessage message)
{
    int poolIndex;
    size_t allocateSize;
    ERR_FAIL_COND_V(false == TryGetPoolIndexForSize(message.BodySize, poolIndex, allocateSize), false);

    std::shared_lock lock{ mutex_ };
    std::lock_guard producerLock{ producerMutex_ };

    QueueMessage queueMessage{};
    queueMessage.SessionId = message.SessionId;
    queueMessage.MessageType = message.MessageType;
    queueMessage.BodySize = message.BodySize;

    if (message.BodySize > 0)
    {
        auto& pool = pools_[currentPushIndex_][poolIndex];

        if (pool.empty())
        {
            const auto beforePoolSize = pool.size();
            for (int i = 0; i < beforePoolSize+1; ++i)
            {
                pool.emplace_back(std::make_unique<char[]>(allocateSize));
            }
        }

        auto block = std::move(pool.back());
        pool.pop_back();

        memcpy_s(block.get(), allocateSize, message.Body, message.BodySize);
        queueMessage.Body = std::move(block);
    }

    auto& pushQueue = queues_[currentPushIndex_];
    pushQueue.emplace(std::move(queueMessage));
    count_.fetch_add(1, std::memory_order_release);
    return true;
}

void GameServerPipe::Pop()
{
    auto& popQueue = queues_[1 - currentPushIndex_];

    if (popQueue.empty())
    {
        return;
    }

    int poolIndex;
    size_t allocateSize;
    ERR_FAIL_COND(false == TryGetPoolIndexForSize(popQueue.front().BodySize, poolIndex, allocateSize));

    auto& pool = pools_[1 - currentPushIndex_][poolIndex];
    pool.emplace_back(std::move(popQueue.front().Body));
    popQueue.pop();
    count_.fetch_sub(1, std::memory_order_release);

    if (popQueue.empty())
    {
        std::unique_lock lock{ mutex_ };
        currentPushIndex_ = 1 - currentPushIndex_;
    }
}