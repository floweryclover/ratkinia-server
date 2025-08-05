//
// Created by floweryclover on 2025-05-05.
//

#include "CtsStub.h"
#include "Database.h"
#include "GameServer.h"
#include <regex>

using namespace pqxx;
using namespace Database;

CtsStub::CtsStub(GameServer& gameServer)
    : gameServer_{ gameServer }
{

}

void CtsStub::OnParseMessageFailed(const uint32_t context,
                                      RatkiniaProtocol::CtsMessageType messageType)
{

}

void CtsStub::OnUnknownMessageType(const uint32_t context,
                                      RatkiniaProtocol::CtsMessageType messageType)
{

}

void CtsStub::OnLoginRequest(const uint32_t context,
                                const std::string& id,
                                const std::string& password)
{
    auto& connection = gameServer_.GetDbConnection();

    nontransaction work{connection};
    const auto res = work.exec(Prepped_FindUserId, id);

    std::array<char, 64> savedPassword{};
    uint32_t pkeyId = 0;
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

    auto loginJob = std::make_unique<LoginJob<GameServer>>(context, pkeyId, password, savedPassword);
    gameServer_.EnqueueAuthJob(std::move(loginJob));
}

void CtsStub::OnRegisterRequest(const uint32_t context,
                                   const std::string& id,
                                   const std::string& password)
{
    if (password.length() < 8 || password.length() > 32)
    {
        gameServer_.GetStcProxy().RegisterResponse(context, false, "비밀번호는 8자 이상 32자 이하여야 합니다.");
        return;
    }

    const std::regex invalidCharactersRegex{R"([^a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?])"};
    if (std::regex_search(password, invalidCharactersRegex))
    {
        gameServer_.GetStcProxy().RegisterResponse(context, false, "비밀번호에 허용되지 않는 문자가 존재합니다.");
        return;
    }

    auto registerJob = std::make_unique<RegisterJob<GameServer>>(context, id, password);
    gameServer_.EnqueueAuthJob(std::move(registerJob));
}