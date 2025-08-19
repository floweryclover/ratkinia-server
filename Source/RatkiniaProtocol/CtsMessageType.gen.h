//
// 2025. 08. 19. 21:16. Ratkinia Protocol Generator에 의해 생성됨.
//

#ifndef RATKINIAPROTOCOL_CTSMESSAGETYPES_GEN_H
#define RATKINIAPROTOCOL_CTSMESSAGETYPES_GEN_H

#include <cstdint>

namespace RatkiniaProtocol 
{
    enum class CtsMessageType : uint16_t
    {
        LoginRequest = 0,
        RegisterRequest = 1,
        CreateCharacter = 2,
        LoadMyCharacters = 3,
    };
}
#endif