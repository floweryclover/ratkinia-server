//
// Created by floweryclover on 2025-05-05.
//

#include "Stub.h"
#include "AuthJob.h"
#include "Database/DatabaseManager.h"
#include "Environment.h"
#include "Proxy.h"
#include "Component/ComponentManager.h"
#include "EntityManager.h"
#include "GlobalObject/GlobalObjectManager.h"

#include "C_NameTag.h"
#include "C_HumanLikeBody.h"

#include "G_Auth.h"
#include "G_Possession.h"
#include "G_PlayerCharacters.h"

#include <regex>

#include "D_Accounts.h"
#include "D_PlayerCharacters.h"

using namespace RatkiniaProtocol;

Stub::Stub(MutableEnvironment& environment)
    : environment_{ environment }
{
}

void Stub::OnParseMessageFailed(const uint32_t context, CtsMessageType messageType)
{
}

void Stub::OnUnknownMessageType(const uint32_t context, CtsMessageType messageType)
{
}

void Stub::OnLoginRequest(const uint32_t context,
                          const std::string& id,
                          const std::string& password)
{
    const auto account = environment_.DatabaseManager.Get<D_Accounts>().TryGetAccountByUserId(id);
    std::array<char, 64> savedPassword;
    uint32_t pkeyId = LoginJob::InvalidId;
    if (account)
    {
        pkeyId = account->Id;
        const auto savedPasswordString = account->Password;
        const size_t copySize = savedPasswordString.size() + 1;
        memcpy(savedPassword.data(), savedPasswordString.c_str(), copySize);
    }
    else
    {
        memcpy(savedPassword.data(), EmptyHashedPassword.c_str(), EmptyHashedPassword.size() + 1);
    }

    environment_.GlobalObjectManager.Get<G_Auth>().CreateAuthJob<LoginJob>(context, pkeyId, password, savedPassword);
}

void Stub::OnRegisterRequest(const uint32_t context,
                             const std::string& id,
                             const std::string& password)
{

    if (id.length() < 6)
    {
        environment_.Proxy.RegisterResponse(context, false, "아이디가 너무 짧습니다.");
        return;
    }
    if (id.length() > 24)
    {
        environment_.Proxy.RegisterResponse(context, false, "아이디가 너무 깁니다.");
        return;
    }
    if (const std::regex invalidAccountRegex{ R"([^a-z0-9_])" };
        std::regex_search(id, invalidAccountRegex))
    {
        environment_.Proxy.RegisterResponse(context, false, "아이디는 영문 소문자, 숫자 또는 언더스코어(_)로만 구성되어야 합니다.");
        return;
    }


    if (password.length() < 8)
    {
        environment_.Proxy.RegisterResponse(context, false, "패스워드가 너무 짧습니다.");
        return;
    }
    if (password.length() > 32)
    {
        environment_.Proxy.RegisterResponse(context, false, "패스워드가 너무 깁니다.");
        return;
    }
    if (const std::regex invalidPasswordRegex{ R"([^a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?])" };
        std::regex_search(password, invalidPasswordRegex))
    {
        environment_.Proxy.RegisterResponse(context, false, "패스워드에 허용되지 않는 문자가 존재합니다.");
        return;
    }
    environment_.GlobalObjectManager.Get<G_Auth>().CreateAuthJob<RegisterJob>(context, id, password);
}

void Stub::OnCreateCharacter(const uint32_t context, const std::string& name)
{
    auto& d_playerCharacters = environment_.DatabaseManager.Get<D_PlayerCharacters>();
    const auto playerId = environment_.GlobalObjectManager.Get<G_Auth>().TryGetPlayerIdOfContext(context);
    if (!playerId)
    {
        environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_UnknownError);
        return;
    }

    const auto [createCharacterResult, characterId] = d_playerCharacters.TryCreatePlayerCharacter(*playerId, name);
    if (createCharacterResult == D_PlayerCharacters::CreateCharacterResult::DuplicateName)
    {
        environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_DuplicateName);
        return;
    }
    if (createCharacterResult == D_PlayerCharacters::CreateCharacterResult::InvalidFormat)
    {
        environment_.Proxy.CreateCharacterResponse(context,
                                                   CreateCharacterResponse_CreateCharacterResult_InvalidNameLength);
        return;
    }

    if (createCharacterResult == D_PlayerCharacters::CreateCharacterResult::UnknownError)
    {
        environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_UnknownError);
        return;
    }

    const auto entity = environment_.EntityManager.Create();

    environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_Success);
    environment_.GlobalObjectManager.Get<G_PlayerCharacters>().AddOwnership(*playerId, characterId, entity);
}

void Stub::OnLoadMyCharacters(const uint32_t context)
{
    auto& d_playerCharacters = environment_.DatabaseManager.Get<D_PlayerCharacters>();
    auto& g_auth = environment_.GlobalObjectManager.Get<G_Auth>();
    const auto playerId = g_auth.TryGetPlayerIdOfContext(context);
    if (!playerId)
    {
        environment_.Proxy.Notify(context, Notify_Type_Fatal, "로그인 세션이 유효하지 않습니다.");
        return;
    }

    const auto characters = d_playerCharacters.TryGetPlayerCharactersByPlayerId(*playerId);
    if (!characters)
    {
        return;
    }

    environment_.Proxy.SendMyCharacters(
        context,
        *characters,
        [](const D_PlayerCharacters::PlayerCharacterRow* const playerCharacter, SendMyCharacters_Data& data)
        {
            data.set_id(playerCharacter->Id);
            data.set_name(playerCharacter->Name);
        });
}

void Stub::OnSelectCharacter(const uint32_t context, const uint64_t id)
{
    auto& g_auth = environment_.GlobalObjectManager.Get<G_Auth>();
    const auto playerId = g_auth.TryGetPlayerIdOfContext(context);
    if (!playerId)
    {
        environment_.Proxy.Notify(context, Notify_Type_Fatal, "로그인이 필요합니다.");
        return;
    }

    auto& g_playerCharacters = environment_.GlobalObjectManager.Get<G_PlayerCharacters>();
    const auto entity = g_playerCharacters.GetEntityOf(*playerId, id);
    if (!entity)
    {
        environment_.Proxy.Notify(context, Notify_Type_Fatal, "요청한 플레이어와 캐릭터에 해당하는 엔티티를 찾을 수 없습니다.");
        return;
    }

    auto& g_possession = environment_.GlobalObjectManager.Get<G_Possession>();
    if (const auto possessionResult = g_possession.TryPossess(context, entity);
        possessionResult != G_Possession::PosessionResult::Success)
    {
        environment_.Proxy.Notify(context, Notify_Type_Fatal, "이미 접속 중입니다.");
        return;
    }

    environment_.Proxy.OpenWorld(context);

    environment_.Proxy.SpawnEntity(
        context,
        environment_.EntityManager,
        [entity](const Entity worldEntity, SpawnEntity_Data& data)
        {
            data.set_type(worldEntity == entity ? SpawnEntity_Type_MyCharacter : SpawnEntity_Type_Normal);
            data.set_entity_id(worldEntity.GetId());
        });

    environment_.Proxy.AttachComponent(
        context,
        environment_.ComponentManager.Components<C_HumanLikeBody>(),
        [](const std::pair<Entity, C_HumanLikeBody>& pair, AttachComponent_Data& data)
        {
            data.set_target_entity(pair.first.GetId());
            data.set_component_runtime_order(C_HumanLikeBody::GetRuntimeOrder());
        });
    static int cnt;
    ++cnt;

    environment_.Proxy.UpdateComponent(
        context,
        environment_.ComponentManager.Components<C_HumanLikeBody>(),
        [](const std::pair<Entity, C_HumanLikeBody>& pair, UpdateComponent_Data& data)
        {
            data.set_target_entity(pair.first.GetId());
            data.mutable_component_variant()->mutable_human_like_body()->set_static_mesh_name(cnt % 2 ? "Normal" : "Cube");
        });
}
