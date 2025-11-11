//
// Created by floweryclover on 2025-11-02.
//

#ifndef ACTORNETWORKINTERFACE_H
#define ACTORNETWORKINTERFACE_H

#include "StcProxy.gen.h"
#include "NetworkServer.h"

class ActorNetworkInterface final : public RatkiniaProtocol::IStcProxy<ActorNetworkInterface>
{
public:
    explicit ActorNetworkInterface(NetworkServer& networkServer);

    template<typename TProtobufMessage>
    void WriteMessage(const uint32_t context, const RatkiniaProtocol::StcMessageType messageType, const TProtobufMessage& message)
    {
        networkServer_->SendMessageTo(context, static_cast<uint16_t>(messageType), message);
    }

    void DisconnectSession(const uint32_t context)
    {
        networkServer_->DisconnectSession(context);
    }

    [[nodiscard]]
    bool TryChangeAssociatedActor(const uint32_t context, const auto& currentActorName, auto&& newActorName)
    {
        return networkServer_->TryChangeAssociatedActor(context, currentActorName, std::forward<decltype(newActorName)>(newActorName));
    }

    void ChangeAssociatedActor(const uint32_t context, auto&& newActorName)
    {
        networkServer_->ChangeAssociatedActor(context, std::forward<decltype(newActorName)>(newActorName));
    }

private:
    NetworkServer* networkServer_;
};

#endif //ACTORNETWORKINTERFACE_H
