//
// Created by floweryclover on 2025-04-28.
//

#include "GameServerChannel.h"
#include "MessagePrinter.h"

GameServerChannel::GameServerChannel()
    : currentPushIndex_{ 0 }
{

}

bool GameServerChannel::Push(PushMessage message)
{
    if (message.BodySize > RatkiniaProtocol::MessageMaxSize - RatkiniaProtocol::MessageHeaderSize)
    {
        return false;
    }


    std::shared_lock lock{ mutex_ };
    std::lock_guard producerLock{ producerMutex_ };

    QueueMessage queueMessage{};
    queueMessage.SessionId = message.SessionId;
    queueMessage.MessageType = message.MessageType;
    queueMessage.BodySize = message.BodySize;

    if (message.BodySize > 0)
    {
        auto& pool = pools_[currentPushIndex_];

        auto block = pool.Acquire(message.BodySize);

        memcpy_s(block, message.BodySize, message.Body, message.BodySize);
        queueMessage.Body = block;
    }

    auto& pushQueue = queues_[currentPushIndex_];
    pushQueue.emplace(queueMessage);
    count_.fetch_add(1, std::memory_order_release);
    return true;
}