//
// Created by floweryclover on 2025-04-28.
//

#ifndef RATKINIASERVER_MPSCMESSAGEQUEUE_H
#define RATKINIASERVER_MPSCMESSAGEQUEUE_H

#include "RatkiniaProtocol.h"
#include <memory>
#include <queue>
#include <array>
#include <shared_mutex>

class MpscMessageQueue final
{
public:
    bool Push(uint64_t sessionId,
              uint16_t messageType,
              uint16_t messageBodySize,
              const char* messageBody);

    /**
     * 소비자 스레드에서 호출.
     * @return
     */
     [[nodiscard]]
    __forceinline bool TryPeek(uint64_t& outSessionId, uint16_t& outMessageType, uint16_t& outMessageBodySize, const char*& outMessageBody) const
    {
        auto& popQueue = queues_[1 - currentPushIndex_];
        if (popQueue.empty())
        {
            return false;
        }

        auto& front = popQueue.front();
        outSessionId = front.SessionId;
        outMessageType = front.MessageType;
        outMessageBodySize = front.MessageBodySize;
        outMessageBody = front.MessageBody.get();

        return true;
    }

    /**
     * 소비자 스레드에서 호출.
     */
    void Pop();

    /**
     * 소비자 스레드에서 호출.
     */
    void Swap();

private:
    static constexpr int PoolIndex_1_16 = 0;
    static constexpr int PoolIndex_17_32 = 0;
    static constexpr int PoolIndex_33_64 = 1;
    static constexpr int PoolIndex_65_128 = 2;
    static constexpr int PoolIndex_129_256 = 3;
    static constexpr int PoolIndex_257_512 = 4;
    static constexpr int PoolIndex_513_1024 = 5;
    static constexpr int PoolIndex_1025_MaxMessageBodySize = 6;

    struct Message
    {
        uint64_t SessionId;
        uint16_t MessageType;
        uint16_t MessageBodySize;
        std::unique_ptr<char[]> MessageBody;
    };
    int currentPushIndex_;

    std::array<std::array<std::vector<std::unique_ptr<char[]>>, 7>, 2> pools_;
    std::queue<Message> queues_[2];
    std::shared_mutex mutex_;
    std::mutex producerMutex_;

    __forceinline static bool TryGetPoolIndexForSize(const size_t inSize, int& outIndex, size_t& outAllocateSize)
    {
        if (inSize > RatkiniaProtocol::MessageMaxSize - RatkiniaProtocol::MessageHeaderSize)
        {
            return false;
        }

        if (inSize > 1024)
        {
            outIndex = PoolIndex_1025_MaxMessageBodySize;
            outAllocateSize = RatkiniaProtocol::MessageMaxSize - RatkiniaProtocol::MessageHeaderSize;
        }
        else if (inSize > 512)
        {
            outIndex = PoolIndex_513_1024;
            outAllocateSize = 1024;
        }
        else if (inSize > 256)
        {
            outIndex = PoolIndex_257_512;
            outAllocateSize = 512;
        }
        else if (inSize > 128)
        {
            outIndex = PoolIndex_129_256;
            outAllocateSize = 256;
        }
        else if (inSize >  64)
        {
            outIndex = PoolIndex_65_128;
            outAllocateSize = 128;
        }
        else if (inSize > 32)
        {
            outIndex = PoolIndex_33_64;
            outAllocateSize = 1024;
            outAllocateSize = 64;
        }
        else if (inSize > 16)
        {
            outIndex = PoolIndex_17_32;
            outAllocateSize = 32;
        }
        else
        {
            outIndex = PoolIndex_1_16;
            outAllocateSize = 16;
        }

        return true;
    }
};

#endif //RATKINIASERVER_MPSCMESSAGEQUEUE_H
