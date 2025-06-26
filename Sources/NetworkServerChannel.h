//
// Created by floweryclover on 2025-05-08.
//

#ifndef RATKINIASERVER_NETWORKSERVERCHANNEL_H
#define RATKINIASERVER_NETWORKSERVERCHANNEL_H

#include "Channel.h"
#include "ScopedNetworkMessage.h"
#include "SpscRingBuffer.h"
#include "RatkiniaProtocol.gen.h"

class NetworkServerChannel final : public CreateSpscChannelFromThis<NetworkServerChannel>
{
public:
    static constexpr size_t BufferCapacity = 65536;

    using PopMessageType = ScopedNetworkMessage<NetworkServerChannel>;

    explicit NetworkServerChannel();

    template<typename TMessage>
    __forceinline bool TryPush(const uint64_t context,
                               const uint16_t messageType,
                               const TMessage& message)
    {
        const auto messageSize{ static_cast<uint16_t>(message.ByteSizeLong()) };
        auto enqueuer{ ringBuffer_.TryGetEnqueuer(8 + 2 + 2 + messageSize) };
        if (!enqueuer) [[unlikely]]
        {
            return false;
        }

        memcpy(enqueuer->EnqueueBuffer, &context, 8);
        memcpy(enqueuer->EnqueueBuffer + 8, &messageType, 2);
        memcpy(enqueuer->EnqueueBuffer + 8 + 2, &messageSize, 2);
        message.SerializeToArray(enqueuer->EnqueueBuffer + 8 + 2 + 2, messageSize);

        return true;
    }

    __forceinline std::optional<PopMessageType> TryPop()
    {
        const auto contextHeader{ ringBuffer_.TryPeek(8 + 2 + 2) };
        if (!contextHeader) [[unlikely]]
        {
            return std::nullopt;
        }

        uint64_t context;
        uint16_t messageType;
        uint16_t bodySize;
        memcpy(&context, *contextHeader, 8);
        memcpy(&messageType, *contextHeader + 8, 2);
        memcpy(&bodySize, *contextHeader + 8 + 2, 2);

        const auto fullBuffer{ ringBuffer_.TryPeek(8 + 2 + 2 + bodySize) };
        if (!fullBuffer) [[unlikely]]
        {
            return std::nullopt;
        }

        return std::make_optional<ScopedNetworkMessage<NetworkServerChannel>>(*this, context, messageType, bodySize, *fullBuffer + 8 + 2 + 2);
    }

    __forceinline void ReleaseScopedMessage(const PopMessageType& message)
    {
        ringBuffer_.Dequeue(8 + 2 + 2 + message.BodySize);
    }

    __forceinline bool Empty()
    {
        const auto [loadedSize, loadedHead, loadedTail]{ ringBuffer_.GetSize() };
        return loadedSize == 0;
    }

private:
    SpscRingBuffer ringBuffer_;
};


#endif //RATKINIASERVER_NETWORKSERVERCHANNEL_H
