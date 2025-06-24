//
// Created by floweryclover on 2025-04-11.
//

#ifndef RATKINIASERVER_SESSION_H
#define RATKINIASERVER_SESSION_H

#include "RatkiniaProtocol.gen.h"
#include "Aligned.h"
#include <WinSock2.h>
#include <memory>
#include <string>
#include <iostream>
enum class IOType : uint8_t
{
    Send,
    Receive,
    Accept
};

struct OverlappedEx final
{
    OVERLAPPED Overlapped{};
    IOType Type{};
};

class Session final
{
public:
    const size_t SessionId;
    const size_t BufferCapacity;

    OverlappedEx IOContext_Receive;
    OverlappedEx IOContext_Send;

    explicit Session(SOCKET socket, size_t sessionId);

    ~Session();

    void Close();

    bool TryAcceptAsync(SOCKET listenSocket, LPOVERLAPPED acceptOverlapped);

    bool TryPostAccept(HANDLE iocpHandle);

    bool TryReceiveAsync();

    bool TrySendAsync();

    bool IsErasable();

    bool TryEnqueueSendBuffer(const char* const data, const size_t size)
    {
        const auto loadedSendBufferHead{ sendBufferHead_->load(std::memory_order_acquire) };
        const auto loadedSendBufferTail{ sendBufferTail_->load(std::memory_order_acquire) };
        const auto sendBufferSize{ loadedSendBufferHead <= loadedSendBufferTail ? loadedSendBufferTail - loadedSendBufferHead : BufferCapacity - loadedSendBufferHead + loadedSendBufferTail };
        const auto sendBufferAvailable{ BufferCapacity - sendBufferSize - 1 };

        if (size > sendBufferAvailable)
        {
            return false;
        }

        const auto primarySize{ std::min<size_t>(size, BufferCapacity - loadedSendBufferTail) };
        const auto secondarySize{ size - primarySize };
        memcpy(sendBuffer_.get() + loadedSendBufferTail, data, primarySize);
        memcpy(sendBuffer_.get(), data + primarySize, secondarySize);
        sendBufferTail_->store((loadedSendBufferTail + size) % BufferCapacity, std::memory_order_release);

        return true;
    }

    void PostReceive(const size_t bytesTransferred)
    {
        receiveFlag_->store(false, std::memory_order_release);
        receiveBufferTail_->store((receiveBufferTail_->load(std::memory_order_acquire) + bytesTransferred) % BufferCapacity);
    }

    void PostSend(const size_t bytesTransferred)
    {
        sendFlag_->store(false, std::memory_order_release);
        sendBufferHead_->store((sendBufferHead_->load(std::memory_order_acquire) + bytesTransferred) % BufferCapacity, std::memory_order_release);
    }

    bool TryPopMessage(auto&& push, auto&& onFailed)
    {
        using namespace RatkiniaProtocol;

        const auto loadedReceiveBufferHead{ receiveBufferHead_->load(std::memory_order_acquire) };
        const auto loadedReceiveBufferTail{ receiveBufferTail_->load(std::memory_order_acquire) };
        const auto receiveBufferSize{ loadedReceiveBufferHead <= loadedReceiveBufferTail ? loadedReceiveBufferTail - loadedReceiveBufferHead : BufferCapacity
                                                                                                                                               - loadedReceiveBufferHead
                                                                                                                                               + loadedReceiveBufferTail };
        if (receiveBufferSize < MessageHeaderSize)
        {
            return false;
        }

        const auto primaryHeaderSize = std::min<size_t>(MessageHeaderSize,
                                                        BufferCapacity
                                                        - loadedReceiveBufferHead);
        const auto secondaryHeaderSize = MessageHeaderSize - primaryHeaderSize;

        MessageHeader header{};
        memcpy_s(&header,
                 primaryHeaderSize,
                 receiveBuffer_.get() + loadedReceiveBufferHead,
                 primaryHeaderSize);
        memcpy_s(reinterpret_cast<char*>(&header) + primaryHeaderSize,
                 secondaryHeaderSize,
                 receiveBuffer_.get(),
                 secondaryHeaderSize);
        header.MessageType = ntohs(header.MessageType);
        header.BodyLength = ntohs(header.BodyLength);

        const auto messageTotalSize = MessageHeaderSize + header.BodyLength;
        if (receiveBufferSize < messageTotalSize)
        {
            return false;
        }

        const auto loadedReceiveBufferHeadAfterHeader = (loadedReceiveBufferHead
                                                         + MessageHeaderSize) % BufferCapacity;
        const auto primaryBodySize = std::min<size_t>(header.BodyLength,
                                                      BufferCapacity
                                                      - loadedReceiveBufferHeadAfterHeader);
        const auto secondaryBodySize = header.BodyLength - primaryBodySize;

        if (primaryBodySize == header.BodyLength)
        {
            if (!push(SessionId,
                      header.MessageType,
                      header.BodyLength,
                      receiveBuffer_.get() + loadedReceiveBufferHeadAfterHeader))
            {
                onFailed(SessionId);
            }
        }
        else
        {
            memcpy_s(receiveTempBuffer_.get(),
                     primaryBodySize,
                     receiveBuffer_.get() + loadedReceiveBufferHeadAfterHeader,
                     primaryBodySize);
            memcpy_s(receiveTempBuffer_.get() + primaryBodySize,
                     secondaryBodySize,
                     receiveBuffer_.get(),
                     secondaryBodySize);
            if (!push(SessionId,
                      header.MessageType,
                      header.BodyLength,
                      receiveTempBuffer_.get()))
            {
                onFailed(SessionId);
            }
        }

        receiveBufferHead_->store((loadedReceiveBufferHeadAfterHeader + header.BodyLength) % BufferCapacity, std::memory_order_release);
        return true;
    }

private:
    SOCKET socket_;
    SOCKADDR_IN addressRaw_;
    std::string address_;

    std::unique_ptr<char[]> receiveBuffer_;
    std::unique_ptr<char[]> receiveTempBuffer_;
    std::unique_ptr<char[]> sendBuffer_;

    AlignedAtomic<size_t> receiveBufferHead_;
    AlignedAtomic<size_t> receiveBufferTail_;
    AlignedAtomic<bool> receiveFlag_;
    AlignedAtomic<size_t> sendBufferHead_;
    AlignedAtomic<size_t> sendBufferTail_;
    AlignedAtomic<bool> sendFlag_;
};


#endif //RATKINIASERVER_SESSION_H
