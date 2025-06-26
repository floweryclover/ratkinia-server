//
// Created by floweryclover on 2025-04-28.
//

#ifndef RATKINIASERVER_GAMESERVERTERMINAL_H
#define RATKINIASERVER_GAMESERVERTERMINAL_H

#include "Channel.h"
#include "Errors.h"
#include "ScopedBufferHandle.h"
#include "MpscMessageBodyPool.h"
#include "RatkiniaProtocol.gen.h"
#include <memory>
#include <queue>
#include <array>
#include <shared_mutex>

class GameServerChannel;

class GameServerChannelMessageDequeuer final
{
public:
    const uint64_t Context;
    const uint16_t MessageType;
    const uint16_t BodySize;
    const char* const Body;

    explicit GameServerChannelMessageDequeuer(const uint64_t context,
                                              const uint16_t messageType,
                                              GameServerChannel* const owner,
                                              char* const buffer,
                                              const size_t bufferSize)
        : Context{ context },
          MessageType{ messageType },
          BodySize{ static_cast<uint16_t>(bufferSize) },
          Body{ buffer },
          RawDequeuer{ owner, buffer, bufferSize }
    {
    }

private:
    const ScopedBufferDequeuer<GameServerChannel> RawDequeuer;
};

class GameServerChannel final : public CreateMpscChannelFromThis<GameServerChannel>
{
public:
    using PopMessageType = GameServerChannelMessageDequeuer;

    explicit GameServerChannel();

    bool Push(const uint64_t context, const uint16_t messageType, const uint16_t bodySize, const char* const body);

    __forceinline void ReleaseDequeueBuffer(const char* const buffer, const size_t bufferSize)
    {
        if (buffer)
        {
            pools_[1 - currentPushIndex_].Release(bufferSize, buffer);
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

        const auto context = popQueueMessage.Context;
        const auto messageType = popQueueMessage.MessageType;
        const auto bodySize = popQueueMessage.BodySize;
        auto body = popQueueMessage.Body;

        popQueue.pop();
        count_.fetch_sub(1, std::memory_order_release);

        return std::make_optional<PopMessageType>(context, messageType, this, body, bodySize);
    }

    __forceinline bool Empty() const
    {
        return count_.load(std::memory_order_acquire) == 0;
    }

private:
    struct QueueMessage final
    {
        uint64_t Context;
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
