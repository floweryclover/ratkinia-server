//
// Created by floweryclover on 2025-04-11.
//

#ifndef RATKINIASERVER_SESSION_H
#define RATKINIASERVER_SESSION_H

#include "ErrorMacros.h"
#include "NetworkServerChannel.h"
#include "RatkiniaProtocol.gen.h"
#include <openssl/ssl.h>
#include <WinSock2.h>
#include <memory>
#include <string>
#include <variant>
#include <optional>

enum class SessionState : uint8_t
{
    PreAccept,
    Ready,
    Closing,
};

enum class IOType : uint8_t
{
    Send,
    Receive,
    Accept,
};

struct OverlappedEx final
{
    OVERLAPPED Overlapped{};
    IOType Type{};
};

inline size_t GetRingBufferSize(const size_t bufferCapacity, const size_t head, const size_t tail)
{
    return head <= tail ? tail - head : bufferCapacity - head + tail;
}

inline std::optional<std::pair<size_t, size_t>> GetWritableSizes(const size_t bufferCapacity, const size_t head, const size_t tail, const size_t sizeToWrite)
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

inline std::optional<std::pair<size_t, size_t>> GetReadableSizes(const size_t bufferCapacity, const size_t head, const size_t tail, const size_t sizeToRead)
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
public:
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

    const size_t SessionId;

    OverlappedEx IOContext_Receive;
    OverlappedEx IOContext_Send;

    explicit Session(HANDLE iocp, SOCKET socket, SSL* ssl, size_t sessionId);

    ~Session();

    void Close();

    bool TryAcceptAsync(SOCKET listenSocket, LPOVERLAPPED acceptOverlapped);

    bool TryPostAccept();

    bool TryReceiveAsync();

    bool TrySendAsync();

    bool IsErasable();

    bool TryEnqueueSendBuffer(const char* data, size_t size);

    bool TryPostReceive(size_t bytesTransferred);

    bool TryPostSend(size_t bytesTransferred);

    void ReleaseReceiveFlag()
    {
        receiveFlag_.store(false, std::memory_order_release);
    }

    void Pop(const size_t size)
    {
        receiveAppBufferHead_ = (receiveAppBufferHead_ + size) % BufferCapacity;
    }

    [[nodiscard]]
    bool IsSendPending() const
    {
        return sendTlsBufferHead_.load(std::memory_order_relaxed) != sendTlsBufferTail_.load(std::memory_order_relaxed) ||
            sendAppBufferHead_.load(std::memory_order_relaxed) != sendAppBufferTail_.load(std::memory_order_relaxed);
    }

    std::optional<MessagePeekResult> TryPeekMessage()
    {
        using namespace RatkiniaProtocol;

        const auto readableHeaderSizes = GetReadableSizes(BufferCapacity, receiveAppBufferHead_, receiveAppBufferTail_, MessageHeaderSize);
        if (!readableHeaderSizes)
        {
            return std::nullopt;
        }
        const auto [primaryHeaderSize, secondaryHeaderSize] = *readableHeaderSizes;

        MessageHeader header;
        memcpy_s(&header,
                 primaryHeaderSize,
                 ReceiveAppBuffer.get() + receiveAppBufferHead_,
                 primaryHeaderSize);
        memcpy_s(reinterpret_cast<char*>(&header) + primaryHeaderSize,
                 secondaryHeaderSize,
                 ReceiveAppBuffer.get(),
                 secondaryHeaderSize);
        header.MessageType = ntohs(header.MessageType);
        header.BodyLength = ntohs(header.BodyLength);

        const auto readableBodySizes = GetReadableSizes(BufferCapacity, (receiveAppBufferHead_ + MessageHeaderSize) % BufferCapacity, receiveAppBufferTail_, header.BodyLength);
        if (!readableBodySizes)
        {
            return std::nullopt;
        }
        const auto [primaryBodySize, secondaryBodySize] = *readableBodySizes;
        const size_t bodyHead = (receiveAppBufferHead_ + MessageHeaderSize) % BufferCapacity;

        if (primaryBodySize == header.BodyLength)
        {
            return std::make_optional<MessagePeekResult>(
                header.MessageType,
                header.BodyLength,
                ReceiveAppBuffer.get() + bodyHead);
        }

        memcpy_s(ReceiveContiguousPopBuffer.get(),
                 primaryBodySize,
                 ReceiveAppBuffer.get() + bodyHead,
                 primaryBodySize);
        memcpy_s(ReceiveContiguousPopBuffer.get() + primaryBodySize,
                 secondaryBodySize,
                 ReceiveAppBuffer.get(),
                 secondaryBodySize);
        return std::make_optional<MessagePeekResult>(
            header.MessageType,
            header.BodyLength,
            ReceiveContiguousPopBuffer.get());
    }

private:
    const HANDLE Iocp;
    const SOCKET Socket;
    SOCKADDR_IN addressRaw_;
    std::string address_;
    std::atomic<SessionState> state_;

    SSL* const Ssl;
    BIO* const WriteBio;
    BIO* const ReadBio;
    std::mutex sslMutex_;

    const std::unique_ptr<char[]> ReceiveTlsBuffer;
    const std::unique_ptr<char[]> ReceiveAppBuffer;
    const std::unique_ptr<char[]> ReceiveContiguousPopBuffer;

    const std::unique_ptr<char[]> SendTlsBuffer;
    const std::unique_ptr<char[]> SendAppBuffer;

    alignas(64) std::atomic<size_t> receiveTlsBufferHead_;
    alignas(64) std::atomic<size_t> receiveTlsBufferTail_;
    alignas(64) std::atomic<size_t> receiveAppBufferHead_;
    alignas(64) std::atomic<size_t> receiveAppBufferTail_;
    alignas(64) std::atomic<bool> receiveFlag_;
    alignas(64) std::mutex receiveBuffersProcessMutex_;

    alignas(64) std::atomic<size_t> sendTlsBufferHead_;
    alignas(64) std::atomic<size_t> sendTlsBufferTail_;
    alignas(64) std::atomic<size_t> sendAppBufferHead_;
    alignas(64) std::atomic<size_t> sendAppBufferTail_;
    alignas(64) std::atomic<bool> sendFlag_;
    alignas(64) std::mutex sendBuffersProcessMutex_;

    SendBuffersProcessResult TryProcessSendBuffers();

    ReceiveBuffersProcessResult TryProcessReceiveBuffers();

    size_t GetReceiveTlsBufferSize() const
    {
        return GetRingBufferSize(BufferCapacity, receiveTlsBufferHead_.load(std::memory_order_relaxed), receiveTlsBufferTail_.load(std::memory_order_relaxed));
    }

    size_t GetReceiveAppBufferSize() const
    {
        return GetRingBufferSize(BufferCapacity, receiveAppBufferHead_.load(std::memory_order_relaxed), receiveAppBufferTail_.load(std::memory_order_relaxed));
    }

    size_t GetSendTlsBufferSize() const
    {
        return GetRingBufferSize(BufferCapacity, sendTlsBufferHead_.load(std::memory_order_relaxed), sendTlsBufferTail_.load(std::memory_order_relaxed));
    }

    size_t GetSendAppBufferSize() const
    {
        return GetRingBufferSize(BufferCapacity, sendAppBufferHead_.load(std::memory_order_relaxed), sendAppBufferTail_.load(std::memory_order_relaxed));
    }
};


#endif //RATKINIASERVER_SESSION_H
