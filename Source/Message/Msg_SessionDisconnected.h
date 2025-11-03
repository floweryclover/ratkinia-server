//
// Created by floweryclover on 2025-11-02.
//

#ifndef MSG_SESSIONDISCONNECTED_H
#define MSG_SESSIONDISCONNECTED_H

#include "Message.h"

struct Msg_SessionDisconnected final : Message<Msg_SessionDisconnected>
{
    uint32_t Context;
};

#endif //MSG_SESSIONDISCONNECTED_H
