//
// 2025. 08. 19. 21:16. Ratkinia Protocol Generator에 의해 생성됨.
//

#ifndef RATKINIAPROTOCOL_STCSTUB_GEN_H
#define RATKINIAPROTOCOL_STCSTUB_GEN_H

#include "StcMessageType.gen.h"
#include "Stc.pb.h"

namespace RatkiniaProtocol 
{
    template<typename TDerivedStub>
    class StcStub
    {
    public:
        virtual ~StcStub() = default;

        virtual void OnUnknownMessageType(const uint32_t context, StcMessageType messageType) = 0;

        virtual void OnParseMessageFailed(const uint32_t context, StcMessageType messageType) = 0;

        virtual void OnDisconnect(const uint32_t context, const std::string& detail) = 0;

        virtual void OnLoginResponse(const uint32_t context, LoginResponse_LoginResult result) = 0;

        virtual void OnRegisterResponse(const uint32_t context, bool successful, const std::string& failedReason) = 0;

        virtual void OnCreateCharacterResponse(const uint32_t context, CreateCharacterResponse_CreateCharacterResult result) = 0;

        virtual void OnSendMyCharacters(const uint32_t context, std::span<const SendMyCharacters_CharacterLoadData*> characterLoadDatas) = 0;

        void HandleStc(const uint32_t context, const uint16_t messageType, const uint16_t bodySize, const char* const body)
        {
            switch (static_cast<int32_t>(messageType))
            {
                case static_cast<int32_t>(StcMessageType::Disconnect):
                {
                    Disconnect DisconnectMessage;
                    if (!DisconnectMessage.ParseFromArray(body, bodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(context, static_cast<StcMessageType>(messageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnDisconnect(context, DisconnectMessage.detail());
                    return;
                }
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
                case static_cast<int32_t>(StcMessageType::CreateCharacterResponse):
                {
                    CreateCharacterResponse CreateCharacterResponseMessage;
                    if (!CreateCharacterResponseMessage.ParseFromArray(body, bodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(context, static_cast<StcMessageType>(messageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnCreateCharacterResponse(context, CreateCharacterResponseMessage.result());
                    return;
                }
                case static_cast<int32_t>(StcMessageType::SendMyCharacters):
                {
                    SendMyCharacters SendMyCharactersMessage;
                    if (!SendMyCharactersMessage.ParseFromArray(body, bodySize))
                    {
                        static_cast<TDerivedStub*>(this)->OnParseMessageFailed(context, static_cast<StcMessageType>(messageType));
                        return;
                    }
                    static_cast<TDerivedStub*>(this)->OnSendMyCharacters(context, std::span<const SendMyCharacters_CharacterLoadData*>{ SendMyCharactersMessage.character_load_datas().data(), SendMyCharactersMessage.character_load_datas().size()});
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