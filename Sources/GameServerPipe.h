//
// Created by floweryclover on 2025-04-28.
//

#ifndef RATKINIASERVER_GAMESERVERTERMINAL_H
#define RATKINIASERVER_GAMESERVERTERMINAL_H

#include "RatkiniaProtocol.gen.h"
#include <memory>
#include <queue>
#include <array>
#include <shared_mutex>

class GameServerPipe final
{
public:
    struct PipeMessage final
    {
        uint64_t SessionId;
        uint16_t MessageType;
        uint16_t BodySize;
        const char* Body;
    };

    using Message = PipeMessage;

    explicit GameServerPipe();

    bool Push(PipeMessage message);

    /**
     * 소비자 스레드에서 호출.
     */
    void Pop();

    /**
     * 소비자 스레드에서 호출.
     * @return
     */
    [[nodiscard]]
    __forceinline bool TryPeek(Message& outMessage) const
    {
        auto& popQueue = queues_[1 - currentPushIndex_];
        if (popQueue.empty())
        {
            return false;
        }

        auto& front = popQueue.front();
        outMessage.SessionId = front.SessionId;
        outMessage.MessageType = front.MessageType;
        outMessage.BodySize = front.BodySize;
        outMessage.Body = front.Body.get();
        return true;
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
        std::unique_ptr<char[]> Body;
    };

    static constexpr int PoolIndex_1_16 = 0;
    static constexpr int PoolIndex_17_32 = 1;
    static constexpr int PoolIndex_33_64 = 2;
    static constexpr int PoolIndex_65_128 = 3;
    static constexpr int PoolIndex_129_256 = 4;
    static constexpr int PoolIndex_257_512 = 5;
    static constexpr int PoolIndex_513_1024 = 6;
    static constexpr int PoolIndex_1025_MaxMessageBodySize = 7;

    int currentPushIndex_;

    std::array<std::array<std::vector<std::unique_ptr<char[]>>, 8>, 2> pools_;
    std::atomic_size_t count_;
    std::queue<QueueMessage> queues_[2];
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
        else if (inSize > 64)
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

    /**
     * 소비자 스레드에서 호출.
     */
    void Swap();
};

#endif //RATKINIASERVER_GAMESERVERTERMINAL_H
