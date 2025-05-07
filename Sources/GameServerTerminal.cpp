//
// Created by floweryclover on 2025-04-28.
//

#include "GameServerTerminal.h"
#include "MessagePrinter.h"

GameServerTerminal::GameServerTerminal()
    : currentPushIndex_{ 0 }
{

}

bool GameServerTerminal::PushMessage(uint64_t sessionId,
                                     uint16_t messageType,
                                     uint16_t messageBodySize,
                                     const char* messageBody)
{
    int poolIndex;
    size_t allocateSize;
    if (!TryGetPoolIndexForSize(messageBodySize, poolIndex, allocateSize))
    {
        return false;
    }
    std::shared_lock lock{ mutex_ };
    std::lock_guard producerLock{ producerMutex_ };

    Message message{};
    message.SessionId = sessionId;
    message.MessageType = messageType;
    message.MessageBodySize = messageBodySize;

    if (messageBodySize > 0)
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

        memcpy_s(block.get(), messageBodySize, messageBody, messageBodySize);
        message.MessageBody = std::move(block);
    }

    auto& pushQueue = queues_[currentPushIndex_];
    pushQueue.emplace(std::move(message));

    return true;
}

void GameServerTerminal::PopMessage()
{
    auto& popQueue = queues_[1 - currentPushIndex_];

    if (popQueue.empty())
    {
        return;
    }

    int poolIndex;
    size_t allocateSize;
    TryGetPoolIndexForSize(popQueue.front().MessageBodySize, poolIndex, allocateSize);

    auto& pool = pools_[1 - currentPushIndex_][poolIndex];
    pool.emplace_back(std::move(popQueue.front().MessageBody));
    popQueue.pop();
}

void GameServerTerminal::SwapQueue()
{
    std::unique_lock lock{ mutex_ };
    currentPushIndex_ = 1 - currentPushIndex_;
}
