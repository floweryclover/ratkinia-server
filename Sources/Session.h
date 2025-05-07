//
// Created by floweryclover on 2025-04-11.
//

#ifndef RATKINIASERVER_SESSION_H
#define RATKINIASERVER_SESSION_H

#include "RatkiniaProtocol.gen.h"
#include <WinSock2.h>
#include <memory>

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
    const SOCKET Socket;
    const size_t SessionId;
    const size_t BufferCapacity;

    OverlappedEx IOContext_Receive;
    OverlappedEx IOContext_Send;

    explicit Session(SOCKET socket, size_t sessionId);

    ~Session();

    bool AcceptAsync(SOCKET listenSocket, LPOVERLAPPED acceptOverlapped);

    bool PostAccept(HANDLE iocpHandle);

    bool ReceiveAsync();

    void PostReceive(size_t bytesTransferred);

    bool TryPopMessage(auto&& push, auto&& onFailed)
    {
        using namespace RatkiniaProtocol;

        const auto receiveBufferSize = receiveBufferSize_;
        if (receiveBufferSize < MessageHeaderSize)
        {
            return false;
        }

        MessageHeader header{};
        const auto receiveBufferBegin_beforeHeader = receiveBufferBegin_;
        const auto primaryHeaderSize = std::min<size_t>(MessageHeaderSize,
                                                        BufferCapacity
                                                        - receiveBufferBegin_beforeHeader);
        const auto secondaryHeaderSize = MessageHeaderSize - primaryHeaderSize;

        memcpy_s(&header,
                 primaryHeaderSize,
                 receiveBuffer_.get() + receiveBufferBegin_beforeHeader,
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

        const auto receiveBufferBegin_afterHeader = (receiveBufferBegin_beforeHeader
                                                     + MessageHeaderSize) % BufferCapacity;
        const auto primaryBodySize = std::min<size_t>(header.BodyLength,
                                                      BufferCapacity
                                                      - receiveBufferBegin_afterHeader);
        const auto secondaryBodySize = header.BodyLength - primaryBodySize;

        if (primaryBodySize == header.BodyLength)
        {
            if (!push(SessionId,
                            header.MessageType,
                            header.BodyLength,
                            receiveBuffer_.get() + receiveBufferBegin_afterHeader))
            {
                onFailed(SessionId);
            }
        }
        else
        {
            memcpy_s(receiveTempBuffer_.get(),
                     primaryBodySize,
                     receiveBuffer_.get() + receiveBufferBegin_afterHeader,
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

        receiveBufferBegin_ = (receiveBufferBegin_afterHeader + header.BodyLength) % BufferCapacity;
        receiveBufferSize_ -= messageTotalSize;
        return true;
    }

private:
    std::unique_ptr<char[]> receiveBuffer_;
    std::unique_ptr<char[]> receiveTempBuffer_;
    size_t receiveBufferBegin_;
    size_t receiveBufferEnd_;
    size_t receiveBufferSize_;

    std::unique_ptr<char[]> sendBuffer_;
    size_t sendBufferBegin_;
    size_t sendBufferEnd_;
};


#endif //RATKINIASERVER_SESSION_H
