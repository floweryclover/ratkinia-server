// Auto-generated from all.desc.

#ifndef RATKINIA_PROTOCOL_H
#define RATKINIA_PROTOCOL_H

#include <cstdint>

namespace RatkiniaProtocol
{
    struct MessageHeader final
    {
        uint16_t MessageType;
        uint16_t BodyLength;
    };

    constexpr size_t MessageMaxSize = 65535 + sizeof(MessageHeader);
    constexpr size_t MessageHeaderSize = sizeof(MessageHeader);

    enum class CtsMessageType : uint16_t
    {
        LoginRequest = 0,
        RegisterRequest = 1,
    };

    enum class StcMessageType : uint16_t
    {
        LoginResponse = 0,
        RegisterResponse = 1,
    };
}

#endif