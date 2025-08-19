//
// Created by floweryclover on 2025-05-05.
//

#include "Stub.h"
#include "AuthJob.h"
#include "Database.h"
#include "G_Auth.h"
#include "Environment.h"
#include "Proxy.h"
#include "GlobalObjectManager.h"
#include <pqxx/pqxx>
#include <regex>

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

    environment_.GlobalObjectManager.Get<G_Auth>()->CreateAuthJob<LoginJob>(context, pkeyId, password, savedPassword);
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

    environment_.GlobalObjectManager.Get<G_Auth>()->CreateAuthJob<RegisterJob>(context, id, password);
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

    const auto id = environment_.GlobalObjectManager.Get<G_Auth>()->TryGetIdOfContext(context);
    if (!id)
    {
        environment_.Proxy.CreateCharacterResponse(context,
                                                   CreateCharacterResponse_CreateCharacterResult_UnknownError);
        return;
    }

    work insertPlayerCharacter{ environment_.DbConnection };
    insertPlayerCharacter.exec(Prepped_CreatePlayerCharacter, params{ *id, name });
    insertPlayerCharacter.commit();

    environment_.Proxy.CreateCharacterResponse(context, CreateCharacterResponse_CreateCharacterResult_Success);
}

void Stub::OnLoadMyCharacters(const uint32_t context)
{
    const auto g_auth = environment_.GlobalObjectManager.Get<G_Auth>();
    const auto id = g_auth->TryGetIdOfContext(context);
    if (!id)
    {
        environment_.Proxy.Disconnect(context, "로그인 세션이 유효하지 않습니다.");
        return;
    }

    nontransaction loadMyCharacters{ environment_.DbConnection };
    const auto result = loadMyCharacters.exec(Prepped_LoadMyCharacters, *id);
    environment_.Proxy.SendMyCharacters(
        context,
        result,
        [](const row& row, SendMyCharacters_CharacterLoadData& data)
        {
            data.set_id(row[0].as<uint32_t>());
            data.set_name(row[2].as<std::string>());
        });
}
