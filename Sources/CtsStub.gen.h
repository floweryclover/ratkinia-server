// Auto-generated from all.desc.

#ifndef CTSSTUB_GEN_H
#define CTSSTUB_GEN_H

#include "RatkiniaProtocol.gen.h"
#include "Cts.pb.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedStub>
    class CtsStub
    {
    public:
        virtual ~CtsStub() = default;

        virtual void OnUnknownMessageType(uint64_t context, CtsMessageType messagetType) = 0;

        virtual void OnParseMessageFailed(uint64_t context, CtsMessageType messagetType) = 0;

        virtual void OnLoginRequest(uint64_t context, const std::string& id, const std::string& hashed_password) = 0;

        virtual void OnRegisterRequest(uint64_t context, const std::string& id, const std::string& hashed_password) = 0;

        void HandleCts(
            const uint64_t context,
            const uint16_t messageType,
            const uint16_t bodySize,
            const char* const body)
        {
            switch (static_cast<int32_t>(messageType))
            {
                case static_cast<int32_t>(CtsMessageType::LoginRequest):
                {
                    LoginRequest LoginRequestMessage;
                    if (!LoginRequestMessage.ParseFromArray(body, bodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(context, static_cast<CtsMessageType>(messageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnLoginRequest(context, LoginRequestMessage.id(), LoginRequestMessage.hashed_password());
                }
                case static_cast<int32_t>(CtsMessageType::RegisterRequest):
                {
                    RegisterRequest RegisterRequestMessage;
                    if (!RegisterRequestMessage.ParseFromArray(body, bodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(context, static_cast<CtsMessageType>(messageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnRegisterRequest(context, RegisterRequestMessage.id(), RegisterRequestMessage.hashed_password());
                }
            }

            static_cast<TDerivedStub*>(this)->OnUnknownMessageType(context, static_cast<CtsMessageType>(messageType));
        }
    };
}
#endif