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
        void RegisterResponse(const uint64_t context, const RegisterResponse_FailedReason failed_reason)
        {
            class RegisterResponse RegisterResponseMessage;
            RegisterResponseMessage.set_failed_reason(failed_reason);
            static_cast<TDerivedProxy*>(this)->WriteMessage(context, StcMessageType::RegisterResponse, RegisterResponseMessage);
        }
    };
}

#endif