//
// Created by floweryclover on 2025-04-28.
//

#ifndef RATKINIASERVER_MPSCMESSAGEQUEUE_H
#define RATKINIASERVER_MPSCMESSAGEQUEUE_H

#include <memory>
#include <queue>
#include <shared_mutex>

class MpscMessageQueue final
{
public:
    void Enqueue(uint64_t sessionId,
                 uint16_t messageType,
                 uint16_t messageBodySize,
                 const char* messageBody);

    /**
     * 소비자 스레드에서 호출.
     */
    void Pop();

    /**
     * 소비자 스레드에서 호출.
     */
    void Swap();

private:
    struct Message
    {
        uint64_t SessionId;
        uint16_t MessageType;
        uint16_t MessageBodySize;
        std::unique_ptr<char[]> MessageBody;
    };
    int currentPushIndex_;

    std::vector<std::unique_ptr<char[]>> pools_[2];
    std::queue<Message> queues_[2];
    std::mutex mutex_;
};

#endif //RATKINIASERVER_MPSCMESSAGEQUEUE_H
