//
// Created by floweryclover on 2025-05-08.
//

#ifndef RATKINIASERVER_NETWORKSERVERCHANNEL_H
#define RATKINIASERVER_NETWORKSERVERCHANNEL_H

#include "Channel.h"
#include "ScopedBufferHandle.h"
#include "SpscRingBuffer.h"
#include "RatkiniaProtocol.gen.h"
#include <iostream>

class NetworkServerChannel;

class NetworkServerChannelMessageDequeuer final
{
public:
    const uint64_t Context;
    const uint16_t MessageType;
    const uint16_t BodySize;
    const char* const Body;

    explicit NetworkServerChannelMessageDequeuer(const uint64_t context,
                                                 const uint16_t messageType,
                                                 NetworkServerChannel* const owner,
                                                 const char* const buffer,
                                                 const size_t bufferSize)
        : Context{ context },
          MessageType{ messageType },
          BodySize{ static_cast<uint16_t>(bufferSize - 8 - 2 - 2) },
          Body{ buffer + 8 + 2 + 2 },
          RawDequeuer{ owner, buffer, bufferSize }
    {
    }

private:
    const ScopedBufferDequeuer<NetworkServerChannel> RawDequeuer;
};

class NetworkServerChannel final : public CreateSpscChannelFromThis<NetworkServerChannel>
{
public:
    static constexpr size_t BufferCapacity = 65536;

    using PopMessageType = NetworkServerChannelMessageDequeuer;

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

        memcpy(enqueuer->Buffer, &context, 8);
        memcpy(enqueuer->Buffer + 8, &messageType, 2);
        memcpy(enqueuer->Buffer + 8 + 2, &messageSize, 2);
        message.SerializeToArray(enqueuer->Buffer + 8 + 2 + 2, messageSize);
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

        return std::make_optional<NetworkServerChannelMessageDequeuer>(context, messageType, this, *fullBuffer, bodySize + 8 + 2 + 2);
    }

    __forceinline void ReleaseDequeueBuffer(const char* const /*buffer*/, const size_t bufferSize)
    {
        ringBuffer_.Dequeue(bufferSize);
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
