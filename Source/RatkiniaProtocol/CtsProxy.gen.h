// 2025. 08. 16. 23:45. Ratkinia Protocol Generator에 의해 생성됨.

#ifndef CTSPROXY_GEN_H
#define CTSPROXY_GEN_H

#include "Cts.pb.h"
#include "RatkiniaProtocol.gen.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedProxy>
    class CtsProxy
    {
    public:
        void LoginRequest(uint32_t context, const std::string& account, const std::string& password)
        {
            class LoginRequest LoginRequestMessage;
            LoginRequestMessage.set_account(account);
            LoginRequestMessage.set_password(password);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::LoginRequest, LoginRequestMessage);
        }

        void RegisterRequest(uint32_t context, const std::string& account, const std::string& password)
        {
            class RegisterRequest RegisterRequestMessage;
            RegisterRequestMessage.set_account(account);
            RegisterRequestMessage.set_password(password);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::RegisterRequest, RegisterRequestMessage);
        }

        void CreateCharacter(uint32_t context, const std::string& name)
        {
            class CreateCharacter CreateCharacterMessage;
            CreateCharacterMessage.set_name(name);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::CreateCharacter, CreateCharacterMessage);
        }
    };
}

#endif