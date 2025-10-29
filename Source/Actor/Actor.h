//
// Created by floweryclover on 2025-10-22.
//

#ifndef ACTOR_H
#define ACTOR_H

#include <variant>
#include <queue>
#include <mutex>
#include <array>
#include <iostream>
#include <syncstream>

class Actor
{
    struct Message
    {
        uint32_t Context;
        uint16_t MessageType;
        uint16_t BodySize;
        std::variant<std::array<char, 256>, std::unique_ptr<char[]>> Body;
    };
public:
    explicit Actor() = default;

    virtual ~Actor() = default;

    Actor(const Actor&) = delete;

    Actor& operator=(const Actor&) = delete;

    Actor(Actor&&) = delete;

    Actor& operator=(Actor&&) = delete;

    void PushMessage(const uint32_t context, const uint16_t messageType, const uint16_t bodySize, const char* const body)
    {
        if (bodySize <= 256)
        {
            std::scoped_lock lock{pushMutex_};
            auto& message = messageQueue_[pushIndex_].emplace();
            message.Context = context;
            message.MessageType = messageType;
            message.BodySize = bodySize;
            memcpy(std::get<0>(message.Body).data(), body, bodySize);
        }
        else
        {
            auto largeBody = std::make_unique<char[]>(bodySize);
            memcpy(largeBody.get(), body, bodySize);

            std::scoped_lock lock{pushMutex_};
            messageQueue_[pushIndex_].emplace(context, messageType, bodySize, std::move(largeBody));
        }
    }

    void HandleAllMessages()
    {
        while (true)
        {
            if (messageQueue_[1 - pushIndex_].empty())
            {
                std::scoped_lock lock{pushMutex_};
                pushIndex_ = 1 - pushIndex_;
            }

            if (messageQueue_[1 - pushIndex_].empty())
            {
                return;
            }

            const auto& [context, messageType, bodySize, body] = messageQueue_[1 - pushIndex_].front();
            OnHandleMessage(
                context,
                messageType,
                bodySize,
                std::holds_alternative<std::array<char, 256>>(body) ? std::get<std::array<char, 256>>(body).data() : std::get<std::unique_ptr<char[]>>(body).get());
            messageQueue_[1 - pushIndex_].pop();
        }
    }

protected:
    virtual void OnHandleMessage(uint32_t context, uint16_t messageType, uint16_t bodySize, const char* body) = 0;

private:
    std::mutex pushMutex_;
    int pushIndex_;
    std::queue<Message> messageQueue_[2];
};

#endif //ACTOR_H
