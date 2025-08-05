//
// Created by floweryclover on 2025-05-08.
//

#ifndef RATKINIASERVER_NETWORKSERVERCHANNEL_H
#define RATKINIASERVER_NETWORKSERVERCHANNEL_H

#include "Channel.h"
#include "SpscRingBuffer.h"
#include "RatkiniaProtocol.gen.h"
#include <iostream>

class NetworkServerChannel final : public CreateSpscChannelFromThis<NetworkServerChannel>
{
public:
    struct PeekOutput final
    {
        uint32_t Context{};
        uint16_t MessageType{};
        uint16_t BodySize{};
        const char* Body;
    };

    static constexpr size_t BufferCapacity = 65536;

    using ChannelPeekOutputType = std::optional<PeekOutput>;
    using ChannelPopInputType = const PeekOutput&;

    explicit NetworkServerChannel();

    template<typename TMessage>
    bool TryPush(const uint32_t context,
                               const uint16_t messageType,
                               const TMessage& message)
    {
        const auto messageSize = static_cast<uint16_t>(message.ByteSizeLong());
        const auto availableSize = ringBuffer_.GetAvailableSize();
        if (availableSize < messageSize) [[unlikely]]
        {
            return false;
        }

        char header[4 + 2 + 2];
        memcpy(header, &context, 4);
        memcpy(header + 4, &messageType, 2);
        memcpy(header + 4 + 2, &messageSize, 2);
        ringBuffer_.TryEnqueue(header, 8);
        ringBuffer_.TryEnqueue(message, messageSize);

        return true;
    }

    ChannelPeekOutputType TryPeek()
    {
        const auto contextHeader{ ringBuffer_.TryPeek(4 + 2 + 2) };
        if (!contextHeader) [[unlikely]]
        {
            return std::nullopt;
        }

        uint32_t context;
        uint16_t messageType;
        uint16_t bodySize;
        memcpy(&context, *contextHeader, 4);
        memcpy(&messageType, *contextHeader + 4, 2);
        memcpy(&bodySize, *contextHeader + 4 + 2, 2);
        const auto fullBuffer{ ringBuffer_.TryPeek(4 + 2 + 2 + bodySize) };
        if (!fullBuffer) [[unlikely]]
        {
            return std::nullopt;
        }

        return std::make_optional<PeekOutput>(context, messageType, bodySize, *fullBuffer + 4 + 2 + 2);
    }

    bool Empty()
    {
        return ringBuffer_.GetSize() == 0;
    }

    void Pop(ChannelPopInputType popInput)
    {
        ringBuffer_.Dequeue(4 + 2 + 2 + popInput.BodySize);
    }

private:
    SpscRingBuffer ringBuffer_;
};


#endif //RATKINIASERVER_NETWORKSERVERCHANNEL_H
