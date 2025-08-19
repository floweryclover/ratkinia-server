//
// 2025. 08. 19. 21:16. Ratkinia Protocol Generator에 의해 생성됨.
//

#ifndef RATKINIAPROTOCOL_CTSPROXY_GEN_H
#define RATKINIAPROTOCOL_CTSPROXY_GEN_H

#include "Cts.pb.h"
#include "CtsMessageType.gen.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedProxy>
    class CtsProxy
    {
    public:
        void LoginRequest(const uint32_t context, std::string account, std::string password)
        {
            class LoginRequest LoginRequestMessage;
            LoginRequestMessage.set_account(account);
            LoginRequestMessage.set_password(password);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::LoginRequest, LoginRequestMessage);
        }

        void RegisterRequest(const uint32_t context, std::string account, std::string password)
        {
            class RegisterRequest RegisterRequestMessage;
            RegisterRequestMessage.set_account(account);
            RegisterRequestMessage.set_password(password);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::RegisterRequest, RegisterRequestMessage);
        }

        void CreateCharacter(const uint32_t context, std::string name)
        {
            class CreateCharacter CreateCharacterMessage;
            CreateCharacterMessage.set_name(name);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::CreateCharacter, CreateCharacterMessage);
        }

        void LoadMyCharacters()
        {
            class LoadMyCharacters LoadMyCharactersMessage;
            static_cast<TDerivedProxy*>(this)->WriteMessage(CtsMessageType::LoadMyCharacters, LoadMyCharactersMessage);
        }
    };
}

#endif