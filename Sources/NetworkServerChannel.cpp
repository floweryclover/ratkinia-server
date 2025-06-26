//
// Created by floweryclover on 2025-05-08.
//

#include "NetworkServerChannel.h"

NetworkServerChannel::NetworkServerChannel()
    : ringBuffer_{ BufferCapacity, RatkiniaProtocol::MessageMaxSize }
{

}
