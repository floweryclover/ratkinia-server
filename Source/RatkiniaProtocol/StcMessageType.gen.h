//
// 2025. 08. 19. 21:16. Ratkinia Protocol Generator에 의해 생성됨.
//

#ifndef RATKINIAPROTOCOL_STCMESSAGETYPES_GEN_H
#define RATKINIAPROTOCOL_STCMESSAGETYPES_GEN_H

#include <cstdint>

namespace RatkiniaProtocol 
{
    enum class StcMessageType : uint16_t
    {
        Disconnect = 0,
        LoginResponse = 1,
        RegisterResponse = 2,
        CreateCharacterResponse = 3,
        SendMyCharacters = 4,
    };
}
#endif