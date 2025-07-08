//
// Created by floweryclover on 2025-04-28.
//

#include "GameServerChannel.h"
#include "MessagePrinter.h"

GameServerChannel::GameServerChannel()
    : currentPushIndex_{ 0 }
{

}

bool GameServerChannel::TryPush(const uint64_t context, const uint16_t messageType, const uint16_t bodySize, const char* const body)
{
    if (bodySize > RatkiniaProtocol::MessageMaxSize - RatkiniaProtocol::MessageHeaderSize)
    {
        return false;
    }

    std::shared_lock lock{ mutex_ };
    std::lock_guard producerLock{ producerMutex_ };
    auto& pushQueue = queues_[currentPushIndex_];
    pushQueue.emplace(context,
                      messageType,
                      bodySize,
                      [&]() -> const char*
                      {
                          if (bodySize == 0)
                          {
                              return nullptr;
                          }
                          auto& pool = pools_[currentPushIndex_];

                          auto block = pool.Acquire(bodySize);
                          ERR_FAIL_NULL_V(block, nullptr);

                          memcpy_s(block, bodySize, body, bodySize);
                          return block;
                      }());

    count_.fetch_add(1, std::memory_order_release);
    return true;
}