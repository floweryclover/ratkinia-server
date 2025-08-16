//
// Created by floweryclover on 2025-04-11.
//

#include "Session.h"
#include "ErrorMacros.h"
#include <openssl/err.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>
#include <openssl/ssl.h>

Session::Session(const HANDLE iocp, const SOCKET socket, SSL* const ssl, const uint32_t context)
    : Iocp{ iocp },
      Context{ context },
      Socket{ socket },
      Ssl{ ssl },
      WriteBio{ BIO_new(BIO_s_mem()) },
      ReadBio{ BIO_new(BIO_s_mem()) },
      ReceiveTlsBuffer{ std::make_unique<char[]>(BufferCapacity) },
      ReceiveAppBuffer{ std::make_unique<char[]>(BufferCapacity) },
      ReceiveContiguousPopBuffer{ std::make_unique<char[]>(RatkiniaProtocol::MessageMaxSize) },
      SendTlsBuffer{ std::make_unique<char[]>(BufferCapacity) },
      SendAppBuffer{ std::make_unique<char[]>(BufferCapacity) },
      SendContiguousPushBuffer{ std::make_unique<char[]>(RatkiniaProtocol::MessageMaxSize) }
{
    CRASH_COND(WriteBio == nullptr);
    CRASH_COND(ReadBio == nullptr);

    ZeroMemory(&IOContext_Receive, sizeof(IOContext_Receive));
    ZeroMemory(&IOContext_Send, sizeof(IOContext_Send));
}

Session::~Session()
{
    if (Socket != INVALID_SOCKET)
    {
        Close();
    }
}

void Session::Close()
{
    if (const auto state = state_.load(std::memory_order_relaxed);
        state != SessionState::PreAccept && state != SessionState::Ready)
    {
        return;
    }

    SSL_shutdown(Ssl);
    SSL_shutdown(Ssl);
    SSL_free(Ssl);
    shutdown(Socket, SD_BOTH);
    closesocket(Socket);

    state_.store(SessionState::Closing, std::memory_order_release);

    MessagePrinter::WriteLine("[접속 해제] ", address_);
}

bool Session::IsIoClear() const
{
    return !sendFlag_.load(std::memory_order_acquire) && !receiveFlag_.load(std::memory_order_acquire);
}

bool Session::TryPostAccept()
{
    if (SSL_accept(Ssl) != 1)
    {
        char buf[256];
        ERR_PRINT_VARARGS("세션 ", Context, " SSL_accept() 에러: ", ERR_error_string(ERR_get_error(), buf));
        return false;
    }
    SSL_set_bio(Ssl, ReadBio, WriteBio);

    if (nullptr
        == CreateIoCompletionPort(reinterpret_cast<HANDLE>(Socket),
                                  Iocp,
                                  reinterpret_cast<ULONG_PTR>(this),
                                  0))
    {
        ERR_PRINT_VARARGS("생성한 ClientSocket의 IOCP 핸들로의 연결 작업에 실패했습니다:", GetLastError());
        return false;
    }

    SOCKADDR_IN* localAddr;
    SOCKADDR_IN* remoteAddr;
    int localAddrLen;
    int remoteAddrLen;

    GetAcceptExSockaddrs(ReceiveTlsBuffer.get(),
                         0,
                         sizeof(SOCKADDR_IN) + 16,
                         sizeof(SOCKADDR_IN) + 16,
                         reinterpret_cast<sockaddr**>(&localAddr),
                         &localAddrLen,
                         reinterpret_cast<sockaddr**>(&remoteAddr),
                         &remoteAddrLen);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &remoteAddr->sin_addr, buf, INET_ADDRSTRLEN))
    {
        address_ = buf;
    }
    else
    {
        address_ = "알 수 없음";
    }
    state_.store(SessionState::Ready, std::memory_order_release);
    MessagePrinter::WriteLine("[접속] ", address_);

    return true;
}

bool Session::TryAcceptAsync(SOCKET listenSocket, LPOVERLAPPED acceptOverlapped)
{
    if (FALSE
        == AcceptEx(listenSocket,
                    Socket,
                    ReceiveTlsBuffer.get(),
                    0,
                    sizeof(SOCKADDR_IN) + 16,
                    sizeof(SOCKADDR_IN) + 16,
                    nullptr,
                    acceptOverlapped))
    {
        const auto lastError = WSAGetLastError();
        if (lastError != ERROR_IO_PENDING)
        {
            ERR_PRINT_VARARGS("AcceptEx() 호출 실패:", lastError);
            return false;
        }
    }

    return true;
}

bool Session::TryReceiveAsync()
{
    if (state_.load(std::memory_order_acquire) != SessionState::Ready)
    {
        return false;
    }

    const size_t receiveEncryptedBufferSize = GetRingBufferSize(BufferCapacity,
                                                                receiveTlsBufferHead_,
                                                                receiveTlsBufferTail_);
    if (receiveEncryptedBufferSize == BufferCapacity - 1)
    {
        return false;
    }

    const size_t writableSize = BufferCapacity - receiveTlsBufferTail_;
    WSABUF wsabuf{};
    wsabuf.buf = ReceiveTlsBuffer.get() + receiveTlsBufferTail_;
    wsabuf.len = writableSize;

    IOContext_Receive.Type = IOType::Receive;

    if (wsabuf.len == 0)
    {
        return false;
    }

    if (receiveFlag_.exchange(true, std::memory_order_acq_rel))
    {
        return true;
    }

    DWORD dwFlags = 0;
    if (0
        != WSARecv(Socket,
                   &wsabuf,
                   1,
                   nullptr,
                   &dwFlags,
                   reinterpret_cast<LPOVERLAPPED>(&IOContext_Receive),
                   nullptr))
    {
        if (const auto errorCode = WSAGetLastError();
            errorCode != WSA_IO_PENDING)
        {
            receiveFlag_.store(false, std::memory_order_release);
            ERR_PRINT_VARARGS("WSARecv() 호출 실패:", errorCode);
            return false;
        }
    }

    return true;
}

bool Session::TrySendAsync()
{
    if (state_.load(std::memory_order_acquire) != SessionState::Ready)
    {
        return false;
    }

    const size_t loadedHead = sendTlsBufferHead_.load(std::memory_order_relaxed);
    const size_t loadedTail = sendTlsBufferTail_.load(std::memory_order_acquire);
    WSABUF wsabuf{};
    wsabuf.buf = SendTlsBuffer.get() + loadedHead;
    wsabuf.len = loadedTail >= loadedHead
                     ? loadedTail - loadedHead
                     : BufferCapacity - loadedHead;

    if (sendFlag_.exchange(true, std::memory_order_acq_rel))
    {
        return true;
    }
    IOContext_Send.Type = IOType::Send;

    if (0
        != WSASend(Socket,
                   &wsabuf,
                   1,
                   nullptr,
                   0,
                   reinterpret_cast<LPOVERLAPPED>(&IOContext_Send),
                   nullptr))
    {
        if (const auto errorCode = WSAGetLastError();
            errorCode != WSA_IO_PENDING)
        {
            sendFlag_.store(false, std::memory_order_release);
            ERR_PRINT_VARARGS("WSASend() 호출 실패:", errorCode);
            return false;
        }
    }

    return true;
}

bool Session::TryPostReceive(const size_t bytesTransferred)
{
    receiveFlag_.store(false, std::memory_order_relaxed);
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

bool Session::TryPostSend(const size_t bytesTransferred)
{
    sendTlsBufferHead_.store((sendTlsBufferHead_.load(std::memory_order_relaxed) + bytesTransferred) % BufferCapacity,
                             std::memory_order_release);

    while (true)
    {
        const auto sendBuffersProcessResult = TryProcessSendBuffers();
        if (sendBuffersProcessResult == SendBuffersProcessResult::Success)
        {
            break;
        }

        if (sendBuffersProcessResult == SendBuffersProcessResult::Failed)
        {
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
            return false;
        }
    }

    sendFlag_.store(false, std::memory_order_relaxed);
    return true;
}

std::optional<Session::MessagePeekResult> Session::TryPeekMessage()
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
    std::lock_guard receiveBuffersProcessLock{ receiveBuffersProcessMutex_ };

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
                std::lock_guard lock{ sslMutex_ };
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
        const size_t loadedHead = receiveAppBufferHead_.load(std::memory_order_acquire);
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
                std::lock_guard lock{ sslMutex_ };
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

Session::SendBuffersProcessResult Session::TryProcessSendBuffers()
{
    std::lock_guard sendBuffersProcessLock{ sendBuffersProcessMutex_ };

    bool wantRead = false;
    {
        // Send App Buffer -> Write Bio
        const size_t loadedHead = sendAppBufferHead_.load(std::memory_order_relaxed);
        const size_t loadedTail = sendAppBufferTail_.load(std::memory_order_acquire);
        size_t totalEncrypted = 0;
        while (true)
        {
            const size_t adjustedHead = (loadedHead + totalEncrypted) % BufferCapacity;
            const size_t encryptableSize = adjustedHead <= loadedTail
                                               ? loadedTail - adjustedHead
                                               : BufferCapacity - adjustedHead;
            if (encryptableSize == 0)
            {
                break;
            }

            const int result = [&]
            {
                std::lock_guard lock{ sslMutex_ };
                return SSL_write(Ssl, SendAppBuffer.get() + adjustedHead, encryptableSize);
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

            totalEncrypted += result;
        }

        if (totalEncrypted > 0)
        {
            sendAppBufferHead_.store((loadedHead + totalEncrypted) % BufferCapacity, std::memory_order_release);
        }
    } // Send App Buffer -> Write Bio

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
                std::lock_guard lock{ sslMutex_ };
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
