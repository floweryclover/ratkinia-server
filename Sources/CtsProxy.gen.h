// Auto-generated from all.desc.

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
        void LoginRequest(const uint32_t context, const std::string& id, const std::string& password)
        {
            class LoginRequest LoginRequestMessage;
            LoginRequestMessage.set_id(id);
            LoginRequestMessage.set_password(password);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::LoginRequest, LoginRequestMessage);
        }

        void RegisterRequest(const uint32_t context, const std::string& id, const std::string& password)
        {
            class RegisterRequest RegisterRequestMessage;
            RegisterRequestMessage.set_id(id);
            RegisterRequestMessage.set_password(password);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::RegisterRequest, RegisterRequestMessage);
        }
    };
}

#endif