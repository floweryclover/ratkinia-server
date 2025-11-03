//
// Created by floweryclover on 2025-04-11.
//

#include "Session.h"
#include "ErrorMacros.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

Session::Session(const SOCKET socket,
                 std::string address,
                 SSL* const ssl,
                 const uint32_t context,
                 std::string initialAssociatedActor)
    : Context{ context },
      AssociatedActor{ std::move(initialAssociatedActor) },
      Socket{ socket },
      Address{ std::move(address) },
      Ssl{ ssl },
      WriteBio{ BIO_new(BIO_s_mem()) },
      ReadBio{ BIO_new(BIO_s_mem()) },
      ReceiveTlsBuffer{ std::make_unique<char[]>(BufferCapacity) },
      ReceiveAppBuffer{ std::make_unique<char[]>(BufferCapacity) },
      ReceiveContiguousPopBuffer{ std::make_unique<char[]>(RatkiniaProtocol::MessageMaxSize) },
      SendTlsBuffer{ std::make_unique<char[]>(BufferCapacity) },
      ioContexts_{},
      isIoOnline_{}
{
    CRASH_COND(WriteBio == nullptr);
    CRASH_COND(ReadBio == nullptr);

    SSL_set_bio(Ssl, ReadBio, WriteBio);

    ioContexts_[IoType_Send].Data.emplace<OverlappedEx::IoData>(IoType_Send, this);
    ioContexts_[IoType_Receive].Data.emplace<OverlappedEx::IoData>(IoType_Receive, this);
    ioContexts_[IoType_InitiateSend].Data.emplace<OverlappedEx::IoData>(IoType_InitiateSend, this);

    std::osyncstream{ std::cout } << "[접속] " << Address << std::endl;
}

Session::~Session()
{
    Cleanup();
    std::osyncstream{ std::cout } << "[접속 해제] " << Address << std::endl;
}

bool Session::TryReceiveAsync()
{
    {
        std::scoped_lock lock{ ioOperationMutexes_[IoType_Receive] };
        if (isIoOnline_[IoType_Receive] ||
            shouldClose_.load(std::memory_order_relaxed))
        {
            return false;
        }
        isIoOnline_[IoType_Receive] = true;
    }

    const size_t loadedTail = receiveTlsBufferTail_.load(std::memory_order_relaxed);
    const size_t loadedHead = receiveTlsBufferHead_.load(std::memory_order_acquire);
    const size_t bufferSize = GetRingBufferSize(BufferCapacity, loadedHead, loadedTail);
    const size_t writableSize = BufferCapacity - loadedTail;

    WSABUF wsabuf;
    wsabuf.buf = ReceiveTlsBuffer.get() + loadedTail;
    wsabuf.len = writableSize;

    if (bufferSize == BufferCapacity - 1 ||
        wsabuf.len == 0)
    {
        std::scoped_lock lock{ ioOperationMutexes_[IoType_Receive] };
        isIoOnline_[IoType_Receive] = false;
        return false;
    }

    DWORD dwFlags = 0;
    if (0
        != WSARecv(Socket,
                   &wsabuf,
                   1,
                   nullptr,
                   &dwFlags,
                   reinterpret_cast<LPOVERLAPPED>(&ioContexts_[IoType_Receive]),
                   nullptr))
    {
        if (const auto errorCode = WSAGetLastError();
            errorCode != WSA_IO_PENDING)
        {
            std::scoped_lock lock{ ioOperationMutexes_[IoType_Receive] };
            isIoOnline_[IoType_Receive] = false;
            ERR_PRINT_VARARGS("WSARecv() 호출 실패:", errorCode);
            return false;
        }
    }

    return true;
}

bool Session::TryPostReceive(const size_t bytesTransferred)
{
    {
        std::scoped_lock lock{ ioOperationMutexes_[IoType_Receive] };
        isIoOnline_[IoType_Receive] = false;
    }

    receiveTlsBufferTail_.store(
        (receiveTlsBufferTail_.load(std::memory_order_relaxed) + bytesTransferred) % BufferCapacity,
        std::memory_order_release);

    while (true)
    {
        const auto receiveBuffersProcessResult = TryProcessReceiveBuffers();
        if (receiveBuffersProcessResult == ReceiveBuffersProcessResult::Success)
        {
            break;
        }

        if (receiveBuffersProcessResult == ReceiveBuffersProcessResult::Failed)
        {
            return false;
        }

        // receiveBuffersProcessResult == ReceiveBuffersProcessResult::WantProcessSendBuffers
        const auto sendBuffersProcessResult = TryProcessSendBuffers();
        if (sendBuffersProcessResult == SendBuffersProcessResult::WantProcessReceiveBuffers)
        {
            break;
        }
        if (sendBuffersProcessResult == SendBuffersProcessResult::Failed)
        {
            return false;
        }
    }
    return true;
}

bool Session::TrySendAsync()
{
    {
        std::scoped_lock lock{ ioOperationMutexes_[IoType_Send] };
        if (isIoOnline_[IoType_Send])
        {
            return true;
        }

        if (shouldClose_.load(std::memory_order_relaxed))
        {
            return false;
        }

        isIoOnline_[IoType_Send] = true;

        const size_t loadedHead = sendTlsBufferHead_.load(std::memory_order_relaxed);
        const size_t loadedTail = sendTlsBufferTail_.load(std::memory_order_acquire);

        const bool isSendAppQueueEmpty = [this]
        {
            std::scoped_lock lock{ sendAppQueueMutex_ };
            return sendAppQueue_.empty();
        }();

        if (loadedHead == loadedTail && isSendAppQueueEmpty)
        {
            isIoOnline_[IoType_Send] = false;
            return true; // 정상 상태임. 보낼 것이 없을 뿐
        }
    }

    while (true)
    {
        const auto sendBuffersProcessResult = TryProcessSendBuffers();
        if (sendBuffersProcessResult == SendBuffersProcessResult::Success)
        {
            break;
        }

        if (sendBuffersProcessResult == SendBuffersProcessResult::Failed)
        {
            std::scoped_lock lock{ ioOperationMutexes_[IoType_Send] };
            isIoOnline_[IoType_Send] = false;
            return false;
        }

        // sendBuffersProcessResult == SendBuffersProcessResult::WantProcessReceiveBuffers
        auto receiveBuffersProcessResult = TryProcessReceiveBuffers();
        if (receiveBuffersProcessResult == ReceiveBuffersProcessResult::WantProcessSendBuffers)
        {
            break;
        }
        if (receiveBuffersProcessResult == ReceiveBuffersProcessResult::Failed)
        {
            std::scoped_lock lock{ ioOperationMutexes_[IoType_Send] };
            isIoOnline_[IoType_Send] = false;
            return false;
        }
    }

    WSABUF wsabuf;
    const size_t loadedHead = sendTlsBufferHead_.load(std::memory_order_relaxed);
    const size_t loadedTail = sendTlsBufferTail_.load(std::memory_order_acquire);

    wsabuf.buf = SendTlsBuffer.get() + loadedHead;
    wsabuf.len = loadedTail >= loadedHead
                     ? loadedTail - loadedHead
                     : BufferCapacity - loadedHead;

    if (0
        != WSASend(Socket,
                   &wsabuf,
                   1,
                   nullptr,
                   0,
                   reinterpret_cast<LPOVERLAPPED>(&ioContexts_[IoType_Send]),
                   nullptr))
    {
        if (const auto errorCode = WSAGetLastError();
            errorCode != WSA_IO_PENDING)
        {
            std::scoped_lock lock{ ioOperationMutexes_[IoType_Send] };
            isIoOnline_[IoType_Send] = false;
            ERR_PRINT_VARARGS("WSASend() 호출 실패:", errorCode);
            return false;
        }
    }

    return true;
}

bool Session::TryPostSend(const size_t bytesTransferred)
{
    {
        std::scoped_lock lock{ ioOperationMutexes_[IoType_Send] };
        isIoOnline_[IoType_Send] = false;
    }

    sendTlsBufferHead_.store((sendTlsBufferHead_.load(std::memory_order_relaxed) + bytesTransferred) % BufferCapacity,
                             std::memory_order_release);

    return true;
}

LPOVERLAPPED Session::InitiateSendAsync()
{
    {
        std::scoped_lock lock{ ioOperationMutexes_[IoType_InitiateSend] };
        if (isIoOnline_[IoType_InitiateSend])
        {
            return nullptr;
        }
        isIoOnline_[IoType_InitiateSend] = true;
    }

    return reinterpret_cast<LPOVERLAPPED>(&ioContexts_[IoType_InitiateSend]);
}

void Session::PostInitiateSend()
{
    std::scoped_lock lock{ ioOperationMutexes_[IoType_InitiateSend] };
    isIoOnline_[IoType_InitiateSend] = false;
}

void Session::PostIoFailed(const uint8_t failedIoType)
{
    std::scoped_lock lock{ ioOperationMutexes_[failedIoType] };
    isIoOnline_[failedIoType] = false;
}

bool Session::Close()
{
    Cleanup();

    std::scoped_lock lock{
        ioOperationMutexes_[IoType_Send], ioOperationMutexes_[IoType_Receive], ioOperationMutexes_[IoType_InitiateSend]
    };
    return !isIoOnline_[IoType_Send] && !isIoOnline_[IoType_Receive] && !isIoOnline_[IoType_InitiateSend];
}

std::optional<Session::MessagePeekResult> Session::PeekReceivedMessage() const
{
    using namespace RatkiniaProtocol;

    const size_t loadedHead = receiveAppBufferHead_.load(std::memory_order_relaxed);
    const size_t loadedTail = receiveAppBufferTail_.load(std::memory_order_acquire);

    const auto readableHeaderSizes = GetReadableSizes(BufferCapacity, loadedHead, loadedTail, MessageHeaderSize);
    if (!readableHeaderSizes)
    {
        return std::nullopt;
    }
    const auto [primaryHeaderSize, secondaryHeaderSize] = *readableHeaderSizes;

    MessageHeader header;
    memcpy_s(&header,
             primaryHeaderSize,
             ReceiveAppBuffer.get() + loadedHead,
             primaryHeaderSize);
    memcpy_s(reinterpret_cast<char*>(&header) + primaryHeaderSize,
             secondaryHeaderSize,
             ReceiveAppBuffer.get(),
             secondaryHeaderSize);
    header.MessageType = ntohs(header.MessageType);
    header.BodySize = ntohs(header.BodySize);

    const auto readableBodySizes = GetReadableSizes(BufferCapacity,
                                                    (loadedHead + MessageHeaderSize) % BufferCapacity,
                                                    loadedTail,
                                                    header.BodySize);
    if (!readableBodySizes)
    {
        return std::nullopt;
    }
    const auto [primaryBodySize, secondaryBodySize] = *readableBodySizes;
    const size_t bodyHead = (loadedHead + MessageHeaderSize) % BufferCapacity;

    if (primaryBodySize == header.BodySize)
    {
        return std::make_optional<MessagePeekResult>(
            header.MessageType,
            header.BodySize,
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
        header.BodySize,
        ReceiveContiguousPopBuffer.get());
}

Session::ReceiveBuffersProcessResult Session::TryProcessReceiveBuffers()
{
    std::scoped_lock receiveBuffersProcessLock{ receiveBuffersProcessMutex_ };

    bool wantSend = false;
    {
        // Receive Tls Buffer -> Read Bio
        const size_t loadedHead = receiveTlsBufferHead_.load(std::memory_order_relaxed);
        const size_t loadedTail = receiveTlsBufferTail_.load(std::memory_order_acquire);

        size_t totalDecrypted = 0;
        while (true)
        {
            const size_t adjustedHead = (loadedHead + totalDecrypted) % BufferCapacity;
            const size_t decryptableSize = adjustedHead <= loadedTail
                                               ? loadedTail - adjustedHead
                                               : BufferCapacity - adjustedHead;
            if (decryptableSize == 0)
            {
                break;
            }

            {
                std::scoped_lock lock{ sslMutex_ };
                CRASH_COND(
                    decryptableSize != BIO_write(ReadBio, ReceiveTlsBuffer.get() + adjustedHead, decryptableSize));
            }

            totalDecrypted += decryptableSize;
        }

        if (totalDecrypted > 0)
        {
            receiveTlsBufferHead_.store((loadedHead + totalDecrypted) % BufferCapacity, std::memory_order_release);
        }
    } // Receive Tls Buffer -> Read Bio

    {
        // Read Bio -> Receive App Buffer
        const size_t loadedTail = receiveAppBufferTail_.load(std::memory_order_relaxed);
        const size_t loadedHead = receiveAppBufferHead_.load(std::memory_order_relaxed);
        const size_t size = loadedHead <= loadedTail
                                ? loadedTail - loadedHead
                                : BufferCapacity - loadedHead + loadedTail;
        const size_t available = BufferCapacity - size - 1;

        size_t totalEnqueued = 0;
        while (true)
        {
            const size_t adjustedTail = (loadedTail + totalEnqueued) % BufferCapacity;
            const size_t adjustedAvailable = available - totalEnqueued;
            if (adjustedAvailable == 0)
            {
                return ReceiveBuffersProcessResult::Failed;
            }
            const size_t enqueuableSize = std::min<size_t>(adjustedAvailable, BufferCapacity - adjustedTail);

            const int result = [&]
            {
                std::scoped_lock lock{ sslMutex_ };
                return SSL_read(Ssl, ReceiveAppBuffer.get() + adjustedTail, enqueuableSize);
            }();
            if (result <= 0)
            {
                const int sslError = SSL_get_error(Ssl, result);
                if (sslError == SSL_ERROR_ZERO_RETURN ||
                    sslError == SSL_ERROR_WANT_READ ||
                    sslError == SSL_ERROR_WANT_WRITE)
                {
                    wantSend = sslError == SSL_ERROR_WANT_WRITE;
                    break;
                }

                char buf[256];
                ERR_PRINT_VARARGS("세션 ",
                                  Context,
                                  " TLS 에러, SSL 에러 코드: ",
                                  sslError,
                                  ", ",
                                  ERR_error_string(ERR_get_error(), buf));
                return ReceiveBuffersProcessResult::Failed;
            }
            totalEnqueued += result;
        }

        if (totalEnqueued > 0)
        {
            receiveAppBufferTail_.store((loadedTail + totalEnqueued) % BufferCapacity, std::memory_order_release);
        }
    } // Read Bio -> Receive App Buffer

    return wantSend ? ReceiveBuffersProcessResult::WantProcessSendBuffers : ReceiveBuffersProcessResult::Success;
}

void Session::Cleanup()
{
    if (shouldClose_.exchange(true, std::memory_order_relaxed))
    {
        return;
    }

    {
        std::scoped_lock lock{ sslMutex_ };
        SSL_shutdown(Ssl);
        SSL_free(Ssl);
    }
    shutdown(Socket, SD_BOTH);
    closesocket(Socket);
}

Session::SendBuffersProcessResult Session::TryProcessSendBuffers()
{
    std::scoped_lock sendBuffersProcessLock{ sendBuffersProcessMutex_ };

    bool wantRead = false;
    {
        // Send App Queue -> Write Bio
        while (true)
        {
            auto [messageSize, message] = [&]() -> std::pair<size_t, std::unique_ptr<char[]>>
            {
                std::scoped_lock lock{ sendAppQueueMutex_ };
                if (sendAppQueue_.empty())
                {
                    return { 0, nullptr };
                }

                auto front = std::move(sendAppQueue_.front());
                sendAppQueue_.pop();
                return std::move(front);
            }();

            if (message == nullptr)
            {
                break;
            }

            const int result = [&]
            {
                std::scoped_lock lock{ sslMutex_ };
                return SSL_write(Ssl, message.get(), messageSize);
            }();
            if (result <= 0)
            {
                const int sslError = SSL_get_error(Ssl, result);
                if (sslError == SSL_ERROR_WANT_READ)
                {
                    wantRead = true;
                    break;
                }

                char buf[256];
                ERR_PRINT_VARARGS("세션 ",
                                  Context,
                                  " TLS 에러, SSL 에러 코드: ",
                                  sslError,
                                  ", ",
                                  ERR_error_string(ERR_get_error(), buf));
                return SendBuffersProcessResult::Failed;
            }
        }
    } // Send App Queue -> Write Bio

    {
        // Write Bio -> Send Tls Buffer
        const size_t loadedHead = sendTlsBufferHead_.load(std::memory_order_acquire);
        const size_t loadedTail = sendTlsBufferTail_.load(std::memory_order_relaxed);
        size_t totalEnqueued = 0;
        while (BIO_pending(WriteBio) > 0)
        {
            const size_t adjustedTail = (loadedTail + totalEnqueued) % BufferCapacity;
            const size_t size = loadedHead <= adjustedTail
                                    ? adjustedTail - loadedHead
                                    : BufferCapacity - loadedHead + adjustedTail;
            const size_t enqueuableSize = std::min<size_t>(BufferCapacity - size - 1, BufferCapacity - adjustedTail);
            if (enqueuableSize == 0)
            {
                return SendBuffersProcessResult::Failed;
            }

            const int result = [&]
            {
                std::scoped_lock lock{ sslMutex_ };
                return BIO_read(WriteBio, SendTlsBuffer.get() + adjustedTail, enqueuableSize);
            }();
            if (result <= 0)
            {
                char buf[256];
                ERR_PRINT_VARARGS("세션 ", Context, " TLS 에러, ", ERR_error_string(ERR_get_error(), buf));
                return SendBuffersProcessResult::Failed;
            }

            totalEnqueued += result;
        }

        if (totalEnqueued > 0)
        {
            sendTlsBufferTail_.store((loadedTail + totalEnqueued) % BufferCapacity, std::memory_order_release);
        }
    } // Write Bio -> Send Tls Buffer

    return wantRead ? SendBuffersProcessResult::WantProcessReceiveBuffers : SendBuffersProcessResult::Success;
}
