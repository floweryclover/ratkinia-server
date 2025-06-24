//
// Created by floweryclover on 2025-05-08.
//

#ifndef RATKINIASERVER_NETWORKSERVERCHANNEL_H
#define RATKINIASERVER_NETWORKSERVERCHANNEL_H

#include "Channel.h"
#include "Aligned.h"
#include "ScopedNetworkMessage.h"
#include "MpscMessageBodyPool.h"
#include "RatkiniaProtocol.gen.h"
#include <atomic>

class NetworkServerChannel final : public CreateSpscChannelFromThis<NetworkServerChannel>
{
public:
    static constexpr size_t RingBufferCapacity = 65536;

    using PopMessageType = ScopedNetworkMessage<NetworkServerChannel>;

    template<typename TMessage>
    bool TryPush(const uint64_t context,
                 const uint16_t messageType,
                 const TMessage& message)
    {
        const auto bodySize{ static_cast<uint16_t>(message.ByteSizeLong()) };
        const auto loadedHead{ head_->load(std::memory_order_acquire) };
        const auto loadedTail{ tail_->load(std::memory_order_acquire) };
        const auto ringBufferSize{ loadedHead <= loadedTail ? loadedTail - loadedHead : loadedTail + RingBufferCapacity - loadedHead };
        const auto availableSize{ RingBufferCapacity - ringBufferSize - 1 };
        if (sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize + bodySize > availableSize)
        {
            return false;
        }

        char contextHeaderBuffer[sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize];
        memcpy(contextHeaderBuffer, &context, sizeof(uint64_t));
        memcpy(contextHeaderBuffer + sizeof(uint64_t), &messageType, sizeof(uint16_t));
        memcpy(contextHeaderBuffer + sizeof(uint64_t) + sizeof(uint16_t), &bodySize, sizeof(uint16_t));

        const auto primaryContextHeaderSize{ std::min(sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize, RingBufferCapacity - loadedTail) };
        const auto secondaryContextHeaderSize{ sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize - primaryContextHeaderSize };
        memcpy(ringBuffer_ + loadedTail, contextHeaderBuffer, primaryContextHeaderSize);
        memcpy(ringBuffer_, contextHeaderBuffer + primaryContextHeaderSize, secondaryContextHeaderSize);

        const auto loadedTailAfterContextHeader{ (loadedTail + sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize) % RingBufferCapacity };
        const auto primaryBodySize{ std::min(static_cast<size_t>(bodySize), RingBufferCapacity - loadedTailAfterContextHeader) };
        const auto secondaryBodySize{ bodySize - primaryBodySize };

        if (secondaryBodySize == 0)
        {
            message.SerializeToArray(ringBuffer_ + loadedTailAfterContextHeader, bodySize);
        }
        else
        {
            message.SerializeToArray(tempPushBuffer_, bodySize);
            memcpy(ringBuffer_ + loadedTailAfterContextHeader, tempPushBuffer_, primaryBodySize);
            memcpy(ringBuffer_, tempPushBuffer_ + primaryContextHeaderSize, secondaryBodySize);
        }

        tail_->store((loadedTailAfterContextHeader + bodySize) % RingBufferCapacity, std::memory_order_release);

        return true;
    }

    std::optional<PopMessageType> TryPop()
    {
        const auto loadedHead{ head_->load(std::memory_order_acquire) };
        const auto loadedTail{ tail_->load(std::memory_order_acquire) };
        const auto size{ loadedHead <= loadedTail ? loadedTail - loadedHead : loadedTail + RingBufferCapacity - loadedHead };

        if (size < sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize)
        {
            return std::nullopt;
        }

        char contextHeaderBuffer[sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize];
        const auto primaryContextHeaderSize{ std::min(sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize, RingBufferCapacity - loadedHead) };
        const auto secondaryContextHeaderSize{ sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize - primaryContextHeaderSize };
        memcpy(contextHeaderBuffer, ringBuffer_ + loadedHead, primaryContextHeaderSize);
        memcpy(contextHeaderBuffer + primaryContextHeaderSize, ringBuffer_, secondaryContextHeaderSize);

        const auto context{ [&]()
                            {
                                uint64_t memcpyContext;
                                memcpy(&memcpyContext, contextHeaderBuffer, sizeof(uint64_t));
                                return memcpyContext;
                            }() };
        const auto messageType{ [&]()
                                {
                                    uint16_t memcpyMessageType;
                                    memcpy(&memcpyMessageType, contextHeaderBuffer + sizeof(uint64_t), sizeof(uint16_t));
                                    return memcpyMessageType;
                                }() };
        const auto bodySize{ [&]()
                             {
                                 uint16_t memcpyBodySize;
                                 memcpy(&memcpyBodySize, contextHeaderBuffer + sizeof(uint64_t) + sizeof(uint16_t), sizeof(uint16_t));
                                 return memcpyBodySize;
                             }() };



        if (size < sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize + bodySize)
        {
            return std::nullopt;
        }

        const auto loadedHeadAfterContextHeader{ (loadedHead + sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize) % RingBufferCapacity };
        const auto primaryBodySize{ std::min(static_cast<size_t>(bodySize), RingBufferCapacity - loadedHeadAfterContextHeader) };
        const auto secondaryBodySize{ bodySize - primaryBodySize };

        if (secondaryBodySize == 0)
        {
            return std::make_optional<PopMessageType>(*this, context, messageType, bodySize, ringBuffer_ + loadedHeadAfterContextHeader);
        }

        memcpy(tempPopBuffer_, ringBuffer_ + loadedHeadAfterContextHeader, primaryBodySize);
        memcpy(tempPopBuffer_ + primaryBodySize, ringBuffer_, secondaryBodySize);

        return std::make_optional<PopMessageType>(*this, context, messageType, bodySize, tempPopBuffer_);
    }

    __forceinline void ReleaseScopedMessage(const PopMessageType& message)
    {
        head_->store((head_->load(std::memory_order_acquire) + sizeof(uint64_t) + RatkiniaProtocol::MessageHeaderSize + message.BodySize) % RingBufferCapacity, std::memory_order_release);
    }

    __forceinline bool Empty()
    {
        return tail_->load(std::memory_order_acquire) == head_->load(std::memory_order_acquire);
    }

private:
    char ringBuffer_[RingBufferCapacity];
    char tempPopBuffer_[RatkiniaProtocol::MessageMaxSize];
    char tempPushBuffer_[RatkiniaProtocol::MessageMaxSize];

    AlignedAtomic<size_t> head_{}; // inclusive
    AlignedAtomic<size_t> tail_{}; // exclusive, head_와 동일하면 empty.
};


#endif //RATKINIASERVER_NETWORKSERVERCHANNEL_H
