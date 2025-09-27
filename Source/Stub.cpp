//
// Created by floweryclover on 2025-05-05.
//

#include "Stub.h"
#include "AuthJob.h"
#include "Database.h"
#include "Environment.h"
#include "Proxy.h"
#include "ComponentManager.h"
#include "EntityManager.h"
#include "GlobalObjectManager.h"

#include "C_NameTag.h"

#include "G_Auth.h"
#include "G_Possession.h"

#include <pqxx/pqxx>
#include <regex>

#include "C_HumanLikeBody.h"
#include "G_PlayerCharacters.h"


using namespace pqxx;
using namespace Database;
using namespace RatkiniaProtocol;

Stub::Stub(MutableEnvironment& environment)
    : environment_{ environment }
{
}

void Stub::OnParseMessageFailed(const uint32_t context,
                                RatkiniaProtocol::CtsMessageType messageType)
{
}

void Stub::OnUnknownMessageType(const uint32_t context,
                                RatkiniaProtocol::CtsMessageType messageType)
{
}

void Stub::OnLoginRequest(const uint32_t context,
                          const std::string& id,
                          const std::string& password)
{
    nontransaction work{ environment_.DbConnection };
    const auto res = work.exec(Prepped_FindUserId, id);

    std::array<char, 64> savedPassword;
    uint32_t pkeyId = LoginJob::InvalidId;
    if (!res.empty())
    {
        pkeyId = res[0][0].as<uint32_t>();
        const auto savedPasswordString = res[0][2].as<std::string>();
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
    if (password.length() < 8 || password.length() > 32)
    {
        environment_.Proxy.RegisterResponse(context, false, "비밀번호는 8자 이상 32자 이하여야 합니다.");
        return;
    }

    const std::regex invalidCharactersRegex{ R"([^a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?])" };
    if (std::regex_search(password, invalidCharactersRegex))
    {
        environment_.Proxy.RegisterResponse(context, false, "비밀번호에 허용되지 않는 문자가 존재합니다.");
        return;
    }

    environment_.GlobalObjectManager.Get<G_Auth>().CreateAuthJob<RegisterJob>(context, id, password);
}

void Stub::OnCreateCharacter(const uint32_t context, const std::string& name)
{
    if (name.length() == 0 || name.length() > 36)
    {
        environment_.Proxy.CreateCharacterResponse(context,
                                                   CreateCharacterResponse_CreateCharacterResult_InvalidNameLength);
        return;
    }

    if (nontransaction findPlayerCharacterByName{ environment_.DbConnection };
        !findPlayerCharacterByName.exec(Prepped_FindPlayerCharacterByName, name).empty())
    {
        environment_.Proxy.CreateCharacterResponse(context,
                                                   CreateCharacterResponse_CreateCharacterResult_DuplicateName);
        return;
    }

    const auto playerId = environment_.GlobalObjectManager.Get<G_Auth>().TryGetIdOfContext(context);
    if (!playerId)
    {
        environment_.Proxy.CreateCharacterResponse(context,
                                                   CreateCharacterResponse_CreateCharacterResult_UnknownError);
        return;
    }

    work insertPlayerCharacter{ environment_.DbConnection };
    const auto results = insertPlayerCharacter.exec(Prepped_CreatePlayerCharacter, params{ *playerId, name });
    insertPlayerCharacter.commit();

    if (results.empty())
    {
        environment_.Proxy.CreateCharacterResponse(context,
                                                   CreateCharacterResponse_CreateCharacterResult_UnknownError);
        return;
    }

    const uint32_t characterId = results[0][0].as<uint32_t>();
    const auto entity = environment_.EntityManager.Create();

    environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_Success);
    environment_.GlobalObjectManager.Get<G_PlayerCharacters>().AddOwnership(*playerId, characterId, entity);
}

void Stub::OnLoadMyCharacters(const uint32_t context)
{
    auto& g_auth = environment_.GlobalObjectManager.Get<G_Auth>();
    const auto id = g_auth.TryGetIdOfContext(context);
    if (!id)
    {
        environment_.Proxy.Notificate(context, Notificate_Type_Fatal, "로그인 세션이 유효하지 않습니다.");
        return;
    }

    nontransaction loadMyCharacters{ environment_.DbConnection };
    const auto result = loadMyCharacters.exec(Prepped_LoadMyCharacters, *id);
    environment_.Proxy.SendMyCharacters(
        context,
        result,
        [](const row& row, SendMyCharacters_Data& data)
        {
            data.set_id(row[0].as<uint32_t>());
            data.set_name(row[2].as<std::string>());
        });
}

void Stub::OnSelectCharacter(const uint32_t context, const uint32_t id)
{
    auto& g_auth = environment_.GlobalObjectManager.Get<G_Auth>();
    const auto playerId = g_auth.TryGetIdOfContext(context);
    if (!playerId)
    {
        environment_.Proxy.Notificate(context, Notificate_Type_Fatal, "로그인이 필요합니다.");
        return;
    }

    auto& g_playerCharacters = environment_.GlobalObjectManager.Get<G_PlayerCharacters>();
    const auto entity = g_playerCharacters.GetEntityOf(*playerId, id);
    if (!entity)
    {
        environment_.Proxy.Notificate(context, Notificate_Type_Fatal, "요청한 플레이어와 캐릭터에 해당하는 엔티티를 찾을 수 없습니다.");
        return;
    }

    auto& g_possession = environment_.GlobalObjectManager.Get<G_Possession>();
    if (const auto possessionResult = g_possession.TryPossess(context, entity);
        possessionResult != G_Possession::PosessionResult::Success)
    {
        environment_.Proxy.Notificate(context, Notificate_Type_Fatal, "이미 접속 중입니다.");
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