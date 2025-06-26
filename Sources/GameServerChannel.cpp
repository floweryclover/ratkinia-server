//
// Created by floweryclover on 2025-04-28.
//

#include "GameServerChannel.h"
#include "MessagePrinter.h"

GameServerChannel::GameServerChannel()
    : currentPushIndex_{ 0 }
{

}

bool GameServerChannel::Push(const uint64_t context, const uint16_t messageType, const uint16_t bodySize, const char* const body)
{
    if (bodySize > RatkiniaProtocol::MessageMaxSize - RatkiniaProtocol::MessageHeaderSize)
    {
        return false;
    }


    std::shared_lock lock{ mutex_ };
    std::lock_guard producerLock{ producerMutex_ };

    QueueMessage queueMessage{};
    queueMessage.Context = context;
    queueMessage.MessageType = messageType;
    queueMessage.BodySize = bodySize;

    if (bodySize > 0)
    {
        auto& pool = pools_[currentPushIndex_];

        auto block = pool.Acquire(bodySize);
        ERR_FAIL_NULL_V(block, false);

        memcpy_s(block, bodySize, body, bodySize);
        queueMessage.Body = block;
    }

    auto& pushQueue = queues_[currentPushIndex_];
    pushQueue.emplace(queueMessage);
    count_.fetch_add(1, std::memory_order_release);
    return true;
}