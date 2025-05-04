#ifndef RATKINIAPROTOCOL_H
#define RATKINIAPROTOCOL_H

#include <cstdint>

// #define CASE(x) case static_cast<int>(x)

namespace RatkiniaProtocol
{
    struct MessageHeader final
    {
        uint16_t MessageType;
        uint16_t BodyLength;
    };

    constexpr size_t MessageMaxSize = 65535 + sizeof(MessageHeader);
    constexpr size_t MessageHeaderSize = sizeof(MessageHeader);

    namespace Cts
    {
        enum class MessageType : uint16_t
        {
            Unknown,
            LoginRequest
        };
    }

    namespace Stc
    {
        enum class MessageType : uint16_t
        {
            Unknown,
            LoginResponse
        };
    }
}

#endif