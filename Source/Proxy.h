//
// Created by floweryclover on 2025-06-13.
//

#ifndef RATKINIASERVER_STCPROXY_H
#define RATKINIASERVER_STCPROXY_H

#include "StcProxy.gen.h"
#include "NetworkServer.h"

class Proxy final : public RatkiniaProtocol::StcProxy<Proxy>
{
public:
    explicit Proxy(NetworkServer& networkServer)
        : networkServer_{networkServer}
    {}

    template<typename TMessage>
    void WriteMessage(const uint64_t context, const RatkiniaProtocol::StcMessageType messageType, const TMessage& message)
    {
        networkServer_.SendMessage(context, static_cast<uint16_t>(messageType), message);
    }

private:
    NetworkServer& networkServer_;
};

#endif //RATKINIASERVER_STCPROXY_H
