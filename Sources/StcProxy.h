//
// Created by floweryclover on 2025-06-13.
//

#ifndef RATKINIASERVER_STCPROXY_H
#define RATKINIASERVER_STCPROXY_H

#include "StcProxy.gen.h"
#include "NetworkServerChannel.h"
#include "MessagePrinter.h"

class StcProxy final : public RatkiniaProtocol::StcProxy<StcProxy>
{
public:
    explicit StcProxy(NetworkServerChannel::SpscSender networkServerSender);

    template<typename TMessage>
    void WriteMessage(const uint64_t context, const RatkiniaProtocol::StcMessageType messageType, const TMessage& message)
    {
        auto elapsedTime{0};

        while (!networkServerSender_.TryPush(context, static_cast<uint16_t>(messageType), message))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
            elapsedTime += 1;
        }

        if (elapsedTime > 0)
        {
            MessagePrinter::WriteErrorLine("NetworkServerSender::TryPush()에서", elapsedTime, "ms만큼 대기했습니다.");
        }
    }

private:
    NetworkServerChannel::SpscSender networkServerSender_;
};

#endif //RATKINIASERVER_STCPROXY_H
