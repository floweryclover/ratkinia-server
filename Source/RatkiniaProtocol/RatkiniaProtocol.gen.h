//
// 2025. 08. 19. 21:16. Ratkinia Protocol Generator에 의해 생성됨.
//

#ifndef RATKINIAPROTOCOL_H
#define RATKINIA_PROTOCOL_H

#include <cstdint>

namespace RatkiniaProtocol
{
#pragma pack(push, 1)
    struct MessageHeader final
    {
        uint16_t MessageType;
        uint16_t BodySize;
    };
#pragma pack(pop)

    constexpr size_t MessageMaxSize = 1024 + sizeof(MessageHeader);
    constexpr size_t MessageHeaderSize = sizeof(MessageHeader);
    constexpr const char* const Version = "20250819.211618";
}

#endif