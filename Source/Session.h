//
// Created by floweryclover on 2025-04-11.
//

#ifndef RATKINIASERVER_SESSION_H
#define RATKINIASERVER_SESSION_H

#include "ErrorMacros.h"
#include "RatkiniaProtocol.gen.h"
#include <memory>
#include <string>
#include <variant>
#include <optional>

typedef void *HANDLE;
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;
typedef UINT_PTR SOCKET;
typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct bio_st BIO;
struct _OVERLAPPED;
using LPOVERLAPPED = _OVERLAPPED*;

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
    char RawOverlapped[32];
    IOType Type;
};

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

    const uint32_t Context;

    OverlappedEx IOContext_Receive;
    OverlappedEx IOContext_Send;

    explicit Session(HANDLE iocp, SOCKET socket, SSL* ssl, uint32_t context);

    ~Session();

    void Close();

    bool TryAcceptAsync(SOCKET listenSocket, LPOVERLAPPED acceptOverlapped);

    bool TryPostAccept();

    bool TryReceiveAsync();

    bool TrySendAsync();

    bool IsIoClear() const;

    bool TryPostReceive(size_t bytesTransferred);

    bool TryPostSend(size_t bytesTransferred);

    void ReleaseReceiveFlag()
    {
        receiveFlag_.store(false, std::memory_order_release);
    }

    void Pop(const size_t size)
    {
        receiveAppBufferHead_.store((receiveAppBufferHead_.load(std::memory_order_relaxed) + size) % BufferCapacity, std::memory_order_release);
    }

    [[nodiscard]]
    bool IsSendPending() const
    {
        return sendTlsBufferHead_.load(std::memory_order_relaxed) != sendTlsBufferTail_.load(std::memory_order_relaxed)
               ||
               sendAppBufferHead_.load(std::memory_order_relaxed) != sendAppBufferTail_.load(std::memory_order_relaxed);
    }

    std::optional<MessagePeekResult> TryPeekMessage();

    template<typename TMessage>
    bool TryPushMessage(uint16_t messageType, TMessage&& message);

private:
    const HANDLE Iocp;
    const SOCKET Socket;
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
    const std::unique_ptr<char[]> SendContiguousPushBuffer;

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

    size_t GetSendAppBufferSize() const
    {
        return GetRingBufferSize(BufferCapacity,
                                 sendAppBufferHead_.load(std::memory_order_relaxed),
                                 sendAppBufferTail_.load(std::memory_order_relaxed));
    }
};

template<typename TMessage>
bool Session::TryPushMessage(const uint16_t messageType, TMessage&& message)
{
    const size_t bodySize = message.ByteSizeLong();
    const size_t size = RatkiniaProtocol::MessageHeaderSize + bodySize;
    const size_t loadedTail = sendAppBufferTail_.load(std::memory_order_relaxed);
    const size_t loadedHead = sendAppBufferHead_.load(std::memory_order_acquire);
    const size_t sendBufferSize = loadedHead <= loadedTail
                                      ? loadedTail - loadedHead
                                      : BufferCapacity - loadedHead + loadedTail;
    const size_t available = BufferCapacity - sendBufferSize - 1;
    if (size > available)
    {
        return false;
    }

    const RatkiniaProtocol::MessageHeader header
    {
        Htons(messageType),
        Htons(static_cast<uint16_t>(bodySize))
    };

    const size_t primarySize = std::min<size_t>(size, BufferCapacity - loadedTail);
    const size_t secondarySize = size - primarySize;

    if (primarySize == size)
    {
        memcpy(SendAppBuffer.get() + loadedTail, &header, RatkiniaProtocol::MessageHeaderSize);
        message.SerializeToArray(SendAppBuffer.get() + loadedTail + RatkiniaProtocol::MessageHeaderSize, bodySize);
    }
    else
    {
        memcpy(SendContiguousPushBuffer.get(), &header, RatkiniaProtocol::MessageHeaderSize);
        message.SerializeToArray(SendContiguousPushBuffer.get() + RatkiniaProtocol::MessageHeaderSize, bodySize);

        memcpy(SendAppBuffer.get() + loadedTail, SendContiguousPushBuffer.get(), primarySize);
        memcpy(SendAppBuffer.get(), SendContiguousPushBuffer.get() + primarySize, secondarySize);
    }

    sendAppBufferTail_.store((loadedTail + size) % BufferCapacity, std::memory_order_release);
    return true;
}


#endif //RATKINIASERVER_SESSION_H
