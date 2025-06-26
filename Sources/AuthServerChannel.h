//
// Created by floweryclover on 2025-06-26.
//

#ifndef RATKINIASERVER_AUTHSERVERCHANNEL_H
#define RATKINIASERVER_AUTHSERVERCHANNEL_H

#include "Channel.h"
#include "SpscRingBuffer.h"

//enum class AuthServerMessageType : uint8_t
//{
//    Login
//};
//
//struct AuthServerMessageHeader final
//{
//    AuthServerMessageType Type;
//    uint64_t Context;
//    size_t BodySize;
//};
//
//class AuthServerChannel final : public CreateSpscChannelFromThis<AuthServerChannel>
//{
//public:
//    using PopMessageType = AuthServerMessage;
//
//    static constexpr size_t BufferCapacity{65536};
//
//    __forceinline bool TryPush(const AuthServerMessageType type, const uint64_t context, const char* data, const size_t dataSize)
//    {
//        auto dataBuffer{std::unique_ptr<char[]>{}};
//        if (dataSize > 0)
//        {
//            dataBuffer = std::make_unique<char[]>(dataSize);
//            memcpy(dataBuffer.get(), data, dataSize);
//        }
//
//        const auto lock{std::lock_guard{mutex_}};
//        queue_.emplace(type, context, std::move(dataBuffer), dataSize);
//        return true;
//    }
//
//    __forceinline std::optional<AuthServerMessage> TryPop()
//    {
//        const auto lock{std::lock_guard{mutex_}};
//
//        if (queue_.empty())
//        {
//            return std::nullopt;
//        }
//
//        auto front{std::move(queue_.front())};
//        queue_.pop();
//        return front;
//    }
//
//    __forceinline bool Empty()
//    {
//        std::lock_guard lock{mutex_};
//        return queue_.empty();
//    }
//
//private:
//    SpscRingBuffer ringBuffer_;
//};


#endif //RATKINIASERVER_AUTHSERVERCHANNEL_H
