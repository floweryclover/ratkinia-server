//
// Created by floweryclover on 2025-05-05.
//

#include "CtsStub.h"
#include "AuthJob.h"
#include "Database.h"
#include "G_Auth.h"
#include "Environment.h"
#include "StcProxy.h"
#include "GlobalObjectManager.h"
#include <pqxx/pqxx>
#include <regex>

using namespace pqxx;
using namespace Database;

CtsStub::CtsStub(MutableEnvironment& environment)
    : environment_{ environment }
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
    nontransaction work{environment_.DbConnection};
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

void CtsStub::OnRegisterRequest(const uint32_t context,
                                   const std::string& id,
                                   const std::string& password)
{
    if (password.length() < 8 || password.length() > 32)
    {
        environment_.StcProxy.RegisterResponse(context, false, "비밀번호는 8자 이상 32자 이하여야 합니다.");
        return;
    }

    const std::regex invalidCharactersRegex{R"([^a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?])"};
    if (std::regex_search(password, invalidCharactersRegex))
    {
        environment_.StcProxy.RegisterResponse(context, false, "비밀번호에 허용되지 않는 문자가 존재합니다.");
        return;
    }

    environment_.GlobalObjectManager.Get<G_Auth>()->CreateAuthJob<RegisterJob>(context, id, password);
}