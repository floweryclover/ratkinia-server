// 2025. 08. 16. 23:45. Ratkinia Protocol Generator에 의해 생성됨.

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

        virtual void OnUnknownMessageType(uint32_t context, CtsMessageType messageType) = 0;

        virtual void OnParseMessageFailed(uint32_t context, CtsMessageType messageType) = 0;

        virtual void OnLoginRequest(uint32_t context, const std::string& account, const std::string& password) = 0;

        virtual void OnRegisterRequest(uint32_t context, const std::string& account, const std::string& password) = 0;

        virtual void OnCreateCharacter(uint32_t context, const std::string& name) = 0;

        void HandleCts(const uint32_t context, const uint16_t messageType, const uint16_t bodySize, const char* const body)
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
                    static_cast<TDerivedStub*>(this)->OnLoginRequest(context, LoginRequestMessage.account(), LoginRequestMessage.password());
                    return;
                }
                case static_cast<int32_t>(CtsMessageType::RegisterRequest):
                {
                    RegisterRequest RegisterRequestMessage;
                    if (!RegisterRequestMessage.ParseFromArray(body, bodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(context, static_cast<CtsMessageType>(messageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnRegisterRequest(context, RegisterRequestMessage.account(), RegisterRequestMessage.password());
                    return;
                }
                case static_cast<int32_t>(CtsMessageType::CreateCharacter):
                {
                    CreateCharacter CreateCharacterMessage;
                    if (!CreateCharacterMessage.ParseFromArray(body, bodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(context, static_cast<CtsMessageType>(messageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnCreateCharacter(context, CreateCharacterMessage.name());
                    return;
                }
                default:
                {
                    static_cast<TDerivedStub*>(this)->OnUnknownMessageType(context, static_cast<CtsMessageType>(messageType));
                    return;
                }
            }

        }
    };
}
#endif