// 2025. 08. 16. 23:45. Ratkinia Protocol Generator에 의해 생성됨.

#ifndef RATKINIA_PROTOCOL_H
#define RATKINIA_PROTOCOL_H

#include <cstdint>

namespace RatkiniaProtocol
{
    struct MessageHeader final
    {
        uint16_t MessageType;
        uint16_t BodySize;
    };

    constexpr size_t MessageMaxSize = 1024 + sizeof(MessageHeader);
    constexpr size_t MessageHeaderSize = sizeof(MessageHeader);
    constexpr const char* const Version = "20250816.234525";

    enum class CtsMessageType : uint16_t
    {
        LoginRequest = 0,
        RegisterRequest = 1,
        CreateCharacter = 2,
    };

    enum class StcMessageType : uint16_t
    {
        LoginResponse = 0,
        RegisterResponse = 1,
        CreateCharacterResponse = 2,
    };
}

#endif