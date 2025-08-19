//
// 2025. 08. 19. 21:16. Ratkinia Protocol Generator에 의해 생성됨.
//

#ifndef RATKINIAPROTOCOL_STCPROXY_GEN_H
#define RATKINIAPROTOCOL_STCPROXY_GEN_H

#include "Stc.pb.h"
#include "StcMessageType.gen.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedProxy>
    class StcProxy
    {
    public:
        void Disconnect(const uint32_t context, std::string detail)
        {
            class Disconnect DisconnectMessage;
            DisconnectMessage.set_detail(detail);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::Disconnect, DisconnectMessage);
        }

        void LoginResponse(const uint32_t context, const LoginResponse_LoginResult result)
        {
            class LoginResponse LoginResponseMessage;
            LoginResponseMessage.set_result(result);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::LoginResponse, LoginResponseMessage);
        }

        void RegisterResponse(const uint32_t context, const bool successful, std::string failedReason)
        {
            class RegisterResponse RegisterResponseMessage;
            RegisterResponseMessage.set_successful(successful);
            RegisterResponseMessage.set_failed_reason(failedReason);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::RegisterResponse, RegisterResponseMessage);
        }

        void CreateCharacterResponse(const uint32_t context, const CreateCharacterResponse_CreateCharacterResult result)
        {
            class CreateCharacterResponse CreateCharacterResponseMessage;
            CreateCharacterResponseMessage.set_result(result);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::CreateCharacterResponse, CreateCharacterResponseMessage);
        }

        void SendMyCharacters(const uint32_t context, auto&& originalCharacterLoadDatasRange, auto&& characterLoadDatasSetter)
        {
            class SendMyCharacters SendMyCharactersMessage;
            for (auto&& originalCharacterLoadDatasElement : originalCharacterLoadDatasRange)
            {
                SendMyCharacters_CharacterLoadData* const NewcharacterLoadDatasElement = SendMyCharactersMessage.add_character_load_datas();
                characterLoadDatasSetter(std::forward<decltype(originalCharacterLoadDatasElement)>(originalCharacterLoadDatasElement), *NewcharacterLoadDatasElement);
            }
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::SendMyCharacters, SendMyCharactersMessage);
        }
    };
}

#endif