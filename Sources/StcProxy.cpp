//
// Created by floweryclover on 2025-06-13.
//

#include "StcProxy.h"

StcProxy::StcProxy(NetworkServerChannel::SpscSender networkServerSender)
    : networkServerSender_{ std::move(networkServerSender) }
{
}