// Auto-generated from all.desc.

#ifndef STCSTUB_GEN_H
#define STCSTUB_GEN_H

#include "RatkiniaProtocol.gen.h"
#include "Stc.pb.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedStub>
    class StcStub
    {
    public:
        virtual ~StcStub() = default;

        virtual void OnUnknownMessageType(uint32_t context, StcMessageType messageType) = 0;

        virtual void OnParseMessageFailed(uint32_t context, StcMessageType messageType) = 0;

        virtual void OnLoginResponse(uint32_t context, const LoginResponse_Result result) = 0;

        virtual void OnRegisterResponse(uint32_t context, const bool successful, const std::string& failed_reason) = 0;

        void HandleStc(
            const uint32_t context,
            const uint16_t messageType,
            const uint16_t bodySize,
            const char* const body)
        {
            switch (static_cast<int32_t>(messageType))
            {
                case static_cast<int32_t>(StcMessageType::LoginResponse):
                {
                    LoginResponse LoginResponseMessage;
                    if (!LoginResponseMessage.ParseFromArray(body, bodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(context, static_cast<StcMessageType>(messageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnLoginResponse(context, LoginResponseMessage.result());
                    return;
                }
                case static_cast<int32_t>(StcMessageType::RegisterResponse):
                {
                    RegisterResponse RegisterResponseMessage;
                    if (!RegisterResponseMessage.ParseFromArray(body, bodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(context, static_cast<StcMessageType>(messageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnRegisterResponse(context, RegisterResponseMessage.successful(), RegisterResponseMessage.failed_reason());
                    return;
                }
                default:
                {
                    static_cast<TDerivedStub*>(this)->OnUnknownMessageType(context, static_cast<StcMessageType>(messageType));
                    return;
                }
            }

        }
    };
}
#endif