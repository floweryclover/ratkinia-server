// Auto-generated from all.desc.

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
        void LoginResponse(const uint32_t context, const LoginResponse_Result result)
        {
            class LoginResponse LoginResponseMessage;
            LoginResponseMessage.set_result(result);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::LoginResponse, LoginResponseMessage);
        }

        void RegisterResponse(const uint32_t context, const bool successful, const std::string& failed_reason)
        {
            class RegisterResponse RegisterResponseMessage;
            RegisterResponseMessage.set_successful(successful);
            RegisterResponseMessage.set_failed_reason(failed_reason);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::RegisterResponse, RegisterResponseMessage);
        }
    };
}

#endif