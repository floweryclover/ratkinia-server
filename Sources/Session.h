//
// Created by floweryclover on 2025-04-11.
//

#ifndef RATKINIASERVER_SESSION_H
#define RATKINIASERVER_SESSION_H

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
    OVERLAPPED Overlapped {};
    IOType Type {};
};

class Session final
{
public:
    struct BufferData final
    {
        char* Pointer;
        size_t Size;
    };

    const SOCKET Socket;
    const size_t BufferSize;

    OverlappedEx IOContext_Receive;
    OverlappedEx IOContext_Send;

    explicit Session(const SOCKET socket, const size_t bufferSize)
        : Socket{ socket },
          BufferSize{ bufferSize },
          receiveBufferBegin_{ 0 },
          receiveBufferEnd_{ 0 },
          sendBufferBegin_{ 0 },
          sendBufferEnd_{ 0 }
    {
        receiveBuffer_ = std::make_unique<char[]>(bufferSize);
        sendBuffer_ = std::make_unique<char[]>(bufferSize);
    }

    BufferData GetFreeReceiveBuffer()
    {
        return { receiveBuffer_.get() + receiveBufferEnd_,
                 BufferSize - receiveBufferEnd_ };
    }

    BufferData GetFreeSendBuffer()
    {
        return { sendBuffer_.get() + sendBufferEnd_,
                 BufferSize - sendBufferEnd_ };
    }

    BufferData GetFilledReceiveBuffer()
    {
        return { receiveBuffer_.get() + receiveBufferBegin_,
                 receiveBufferEnd_ - receiveBufferBegin_ };
    }

    BufferData GetFilledSendBuffer()
    {
        return { sendBuffer_.get() + sendBufferBegin_,
                 sendBufferEnd_ - sendBufferBegin_ };
    }

private:
    std::unique_ptr<char[]> receiveBuffer_;
    size_t receiveBufferBegin_;
    size_t receiveBufferEnd_;

    std::unique_ptr<char[]> sendBuffer_;
    size_t sendBufferBegin_;
    size_t sendBufferEnd_;

};


#endif //RATKINIASERVER_SESSION_H
