//
// Created by floweryclover on 2025-04-28.
//

#ifndef RATKINIASERVER_GAMESERVERTERMINAL_H
#define RATKINIASERVER_GAMESERVERTERMINAL_H

#include "Channel.h"
#include "Errors.h"
#include "ScopedNetworkMessage.h"
#include "MpscMessageBodyPool.h"
#include "RatkiniaProtocol.gen.h"
#include <memory>
#include <queue>
#include <array>
#include <shared_mutex>

class GameServerChannel final : public CreateMpscChannelFromThis<GameServerChannel>
{
public:
    struct PushMessage final
    {
        uint64_t SessionId;
        uint16_t MessageType;
        uint16_t BodySize;
        const char* Body;
    };

    using PopMessageType = ScopedNetworkMessage<GameServerChannel>;

    explicit GameServerChannel();

    bool Push(PushMessage message);

    __forceinline void ReleaseScopedMessage(const PopMessageType& message)
    {
        if (message.Body)
        {
            pools_[1 - currentPushIndex_].Release(message.BodySize, message.Body);
        }
    }

    /**
     * 소비자 스레드에서 호출.
     * @return
     */
    [[nodiscard]]
    __forceinline std::optional<PopMessageType> TryPop()
    {
        auto& popQueue = queues_[1 - currentPushIndex_];
        if (popQueue.empty())
        {
            std::unique_lock lock{ mutex_ };
            currentPushIndex_ = 1 - currentPushIndex_;
            return std::nullopt;
        }
        auto& popQueueMessage = popQueue.front();

        const auto sessionId = popQueueMessage.SessionId;
        const auto messageType = popQueueMessage.MessageType;
        const auto bodySize = popQueueMessage.BodySize;
        auto body = popQueueMessage.Body;

        popQueue.pop();
        count_.fetch_sub(1, std::memory_order_release);

        return std::make_optional<PopMessageType>(*this, sessionId, messageType, bodySize, body);
    }

    __forceinline bool Empty() const
    {
        return count_.load(std::memory_order_acquire) == 0;
    }

private:
    struct QueueMessage final
    {
        uint64_t SessionId;
        uint16_t MessageType;
        uint16_t BodySize;
        char* Body;
    };

    int currentPushIndex_;

    std::atomic_size_t count_;
    MpscMessageBodyPool pools_[2];
    std::queue<QueueMessage> queues_[2];
    std::shared_mutex mutex_;
    std::mutex producerMutex_;
};

#endif //RATKINIASERVER_GAMESERVERTERMINAL_H
