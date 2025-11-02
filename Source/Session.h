//
// Created by floweryclover on 2025-04-11.
//

#ifndef RATKINIASERVER_SESSION_H
#define RATKINIASERVER_SESSION_H

#include "ErrorMacros.h"
#include "OverlappedEx.h"
#include "RatkiniaProtocol.gen.h"
#include <memory>
#include <optional>
#include <array>
#include <queue>

struct ssl_st;
struct ssl_ctx_st;
struct bio_st;
struct _OVERLAPPED;

using UINT_PTR = unsigned __int64;
using PUINT_PTR = UINT_PTR*;
using SOCKET = UINT_PTR;
using SSL = ssl_st;
using SSL_CTX = ssl_ctx_st;
using BIO = bio_st;
using LPOVERLAPPED = _OVERLAPPED*;
using HANDLE = void*;

inline uint16_t Htons(const uint16_t value)
{
    return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
}

inline size_t GetRingBufferSize(const size_t bufferCapacity, const size_t head, const size_t tail)
{
    return head <= tail ? tail - head : bufferCapacity - head + tail;
}

inline std::optional<std::pair<size_t, size_t>> GetWritableSizes(const size_t bufferCapacity,
                                                                 const size_t head,
                                                                 const size_t tail,
                                                                 const size_t sizeToWrite)
{
    const size_t ringBufferSize = GetRingBufferSize(bufferCapacity, head, tail);
    const size_t ringBufferAvailableSize = bufferCapacity - ringBufferSize - 1;
    if (ringBufferAvailableSize < sizeToWrite)
    {
        return std::nullopt;
    }

    const size_t primarySize = std::min<size_t>(sizeToWrite, bufferCapacity - tail);
    const size_t secondarySize = sizeToWrite - primarySize;
    return std::make_optional<std::pair<size_t, size_t>>(primarySize, secondarySize);
}

inline std::optional<std::pair<size_t, size_t>> GetReadableSizes(const size_t bufferCapacity,
                                                                 const size_t head,
                                                                 const size_t tail,
                                                                 const size_t sizeToRead)
{
    const size_t ringBufferSize = GetRingBufferSize(bufferCapacity, head, tail);
    if (ringBufferSize < sizeToRead)
    {
        return std::nullopt;
    }

    const size_t primarySize = std::min<size_t>(sizeToRead, bufferCapacity - head);
    const size_t secondarySize = sizeToRead - primarySize;
    return std::make_optional<std::pair<size_t, size_t>>(primarySize, secondarySize);
}

class alignas(64) Session final
{
    enum class ReceiveBuffersProcessResult : uint8_t
    {
        Success,
        WantProcessSendBuffers,
        Failed,
    };

    enum class SendBuffersProcessResult : uint8_t
    {
        Success,
        WantProcessReceiveBuffers,
        Failed
    };

    struct MessagePeekResult final
    {
        uint16_t MessageType;
        uint16_t BodySize;
        const char* Body;
    };

    static constexpr size_t BufferCapacity = RatkiniaProtocol::MessageMaxSize * 4;

public:
    constexpr static uint8_t IoType_Send = 0;
    constexpr static uint8_t IoType_Receive = 1;
    constexpr static uint8_t IoType_InitiateSend = 2;

    const uint32_t Context;

    uint32_t AssociatedActor;

    explicit Session(SOCKET socket, std::string address, SSL* ssl, uint32_t context, uint32_t initialAssociatedActor);

    ~Session();

    [[nodiscard]]
    bool TryReceiveAsync();

    [[nodiscard]]
    bool TryPostReceive(size_t bytesTransferred);

    [[nodiscard]]
    bool TrySendAsync();

    [[nodiscard]]
    bool TryPostSend(size_t bytesTransferred);

    [[nodiscard]]
    LPOVERLAPPED InitiateSendAsync();

    void PostInitiateSend();

    void PostIoFailed(uint8_t failedIoType);

    bool Close();

    void PopReceivedMessage(const size_t size)
    {
        receiveAppBufferHead_.store((receiveAppBufferHead_.load(std::memory_order_relaxed) + size) % BufferCapacity,
                                    std::memory_order_release);
    }

    [[nodiscard]]
    std::optional<MessagePeekResult> PeekReceivedMessage();

    template<typename TProtobufMessage>
    void PushMessage(uint16_t messageType, const TProtobufMessage& message);

private:
    const SOCKET Socket;
    const std::string Address;

    SSL* const Ssl;
    BIO* const WriteBio;
    BIO* const ReadBio;
    std::mutex sslMutex_;

    const std::unique_ptr<char[]> ReceiveTlsBuffer;
    const std::unique_ptr<char[]> ReceiveAppBuffer;
    const std::unique_ptr<char[]> ReceiveContiguousPopBuffer;

    const std::unique_ptr<char[]> SendTlsBuffer;

    alignas(64) std::array<OverlappedEx, 3> ioContexts_;
    alignas(64) std::array<bool, 3> isIoOnline_;
    alignas(64) std::array<std::mutex, 3> ioOperationMutexes_;
    alignas(64) std::atomic_bool shouldClose_;

    alignas(64) std::atomic<size_t> receiveTlsBufferHead_;
    alignas(64) std::atomic<size_t> receiveTlsBufferTail_;
    alignas(64) std::atomic<size_t> receiveAppBufferHead_;
    alignas(64) std::atomic<size_t> receiveAppBufferTail_;
    alignas(64) std::mutex receiveBuffersProcessMutex_;

    alignas(64) std::atomic<size_t> sendTlsBufferHead_;
    alignas(64) std::atomic<size_t> sendTlsBufferTail_;
    alignas(64) std::mutex sendBuffersProcessMutex_;
    alignas(64) std::queue<std::pair<size_t, std::unique_ptr<char[]>>> sendAppQueue_;
    alignas(64) std::mutex sendAppQueueMutex_;

    SendBuffersProcessResult TryProcessSendBuffers();

    ReceiveBuffersProcessResult TryProcessReceiveBuffers();

    size_t GetReceiveTlsBufferSize() const
    {
        return GetRingBufferSize(BufferCapacity,
                                 receiveTlsBufferHead_.load(std::memory_order_relaxed),
                                 receiveTlsBufferTail_.load(std::memory_order_relaxed));
    }

    size_t GetReceiveAppBufferSize() const
    {
        return GetRingBufferSize(BufferCapacity,
                                 receiveAppBufferHead_.load(std::memory_order_relaxed),
                                 receiveAppBufferTail_.load(std::memory_order_relaxed));
    }

    size_t GetSendTlsBufferSize() const
    {
        return GetRingBufferSize(BufferCapacity,
                                 sendTlsBufferHead_.load(std::memory_order_relaxed),
                                 sendTlsBufferTail_.load(std::memory_order_relaxed));
    }

    void Cleanup();
};

template<typename TProtobufMessage>
void Session::PushMessage(const uint16_t messageType, const TProtobufMessage& message)
{
    using namespace RatkiniaProtocol;

    auto serializedMessage = std::make_unique<char[]>(sizeof(MessageHeaderSize) + message.ByteSizeLong());

    const MessageHeader header{Htons(messageType), Htons(message.ByteSizeLong())};
    memcpy(serializedMessage.get(), &header, sizeof(MessageHeader));
    message.SerializeToArray(serializedMessage.get() + sizeof(MessageHeader), message.ByteSizeLong());

    {
        std::scoped_lock lock{sendAppQueueMutex_};
        sendAppQueue_.emplace(sizeof(MessageHeader) + message.ByteSizeLong(), std::move(serializedMessage));
    }
}

#endif //RATKINIASERVER_SESSION_H
