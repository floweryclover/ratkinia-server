//
// Created by floweryclover on 2025-04-28.
//

#ifndef RATKINIASERVER_GAMESERVERTERMINAL_H
#define RATKINIASERVER_GAMESERVERTERMINAL_H

#include "Channel.h"
#include "Errors.h"
#include "MpscMessageBodyPool.h"
#include "RatkiniaProtocol.gen.h"
#include <memory>
#include <queue>
#include <array>
#include <shared_mutex>

class GameServerChannel final : public CreateMpscChannelFromThis<GameServerChannel>
{
public:
    struct QueueMessage final
    {
        const uint64_t Context{};
        const uint16_t MessageType{};
        const uint16_t BodySize{};
        const char* const Body{};
    };

    using ChannelPeekOutputType = std::optional<QueueMessage>;
    using ChannelPopInputType = const QueueMessage&;

    explicit GameServerChannel();

    bool TryPush(const uint64_t context, const uint16_t messageType, const uint16_t bodySize, const char* const body);

    /**
     * 소비자 스레드에서 호출.
     * @return
     */
    [[nodiscard]]
    __forceinline ChannelPeekOutputType TryPeek()
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

        return std::make_optional<QueueMessage>(context, messageType, bodySize, body);
    }

    __forceinline void Pop(ChannelPopInputType popInput)
    {
        if (popInput.Body)
        {
            pools_[1 - currentPushIndex_].Release(popInput.BodySize, popInput.Body);
        }
        queues_[1 - currentPushIndex_].pop();
        count_.fetch_sub(1, std::memory_order_release);
    }

    __forceinline bool Empty() const
    {
        return count_.load(std::memory_order_acquire) == 0;
    }

private:

    int currentPushIndex_;

    MpscMessageBodyPool pools_[2];
    std::queue<QueueMessage> queues_[2];
    std::shared_mutex mutex_;
    std::mutex producerMutex_;

    alignas(64) std::atomic_size_t count_;
};

#endif //RATKINIASERVER_GAMESERVERTERMINAL_H
