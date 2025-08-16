// 2025. 08. 16. 23:45. Ratkinia Protocol Generator에 의해 생성됨.

#ifndef STCPROXY_GEN_H
#define STCPROXY_GEN_H

#include "Stc.pb.h"
#include "RatkiniaProtocol.gen.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedProxy>
    class StcProxy
    {
    public:
        void LoginResponse(uint32_t context, const LoginResponse_LoginResult result)
        {
            class LoginResponse LoginResponseMessage;
            LoginResponseMessage.set_result(result);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::LoginResponse, LoginResponseMessage);
        }

        void RegisterResponse(uint32_t context, const bool successful, const std::string& failedReason)
        {
            class RegisterResponse RegisterResponseMessage;
            RegisterResponseMessage.set_successful(successful);
            RegisterResponseMessage.set_failed_reason(failedReason);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::RegisterResponse, RegisterResponseMessage);
        }

        void CreateCharacterResponse(uint32_t context, const CreateCharacterResponse_CreateCharacterResult successful)
        {
            class CreateCharacterResponse CreateCharacterResponseMessage;
            CreateCharacterResponseMessage.set_successful(successful);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::CreateCharacterResponse, CreateCharacterResponseMessage);
        }
    };
}

#endif