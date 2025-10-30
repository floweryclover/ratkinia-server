//
// Created by floweryclover on 2025-10-30.
//

#ifndef MSG_CTS_H
#define MSG_CTS_H

#include "Message.h"

struct Msg_Cts final : Message<Msg_Cts>
{
    uint32_t Context;
    uint16_t MessageType;
    uint16_t BodySize;
    std::unique_ptr<char[]> Body;
};

#endif //MSG_CTS_H
