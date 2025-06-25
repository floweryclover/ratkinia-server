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
        void LoginRequest(const uint64_t context, const std::string& id, const std::string& hashed_password)
        {
            class LoginRequest LoginRequestMessage;
            LoginRequestMessage.set_id(id);
            LoginRequestMessage.set_hashed_password(hashed_password);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::LoginRequest, LoginRequestMessage);
        }

        void RegisterRequest(const uint64_t context, const std::string& id, const std::string& hashed_password)
        {
            class RegisterRequest RegisterRequestMessage;
            RegisterRequestMessage.set_id(id);
            RegisterRequestMessage.set_hashed_password(hashed_password);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, CtsMessageType::RegisterRequest, RegisterRequestMessage);
        }
    };
}

#endif