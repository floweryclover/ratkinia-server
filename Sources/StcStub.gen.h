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

        virtual void OnUnknownMessageType(uint64_t context, StcMessageType messagetType) = 0;

        virtual void OnParseMessageFailed(uint64_t context, StcMessageType messagetType) = 0;

        virtual void OnLoginResponse(uint64_t context, const bool successful, const std::string& failure_reason) = 0;

        void HandleStc(
            const uint64_t context,
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
                    static_cast<TDerivedStub*>(this)->OnLoginResponse(context, LoginResponseMessage.successful(), LoginResponseMessage.failure_reason());
                }
            }

            static_cast<TDerivedStub*>(this)->OnUnknownMessageType(context, static_cast<StcMessageType>(messageType));
        }
    };
}
#endif