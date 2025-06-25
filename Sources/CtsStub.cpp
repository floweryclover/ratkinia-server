//
// Created by floweryclover on 2025-05-05.
//

#include "CtsStub.h"
#include "GameServer.h"
#include "Errors.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <iostream>

using namespace pqxx;

void Sha512(const char* inText, char* outHashed129);

CtsStub::CtsStub(GameServer& gameServer)
    : gameServer_{ gameServer }
{

}

void CtsStub::OnParseMessageFailed(uint64_t context,
                                      RatkiniaProtocol::CtsMessageType messageType)
{

}

void CtsStub::OnUnknownMessageType(uint64_t context,
                                      RatkiniaProtocol::CtsMessageType messageType)
{

}

void CtsStub::OnLoginRequest(uint64_t context,
                                const std::string& id,
                                const std::string& hashed_password)
{
    char doubleHashedPassword[129];
    Sha512(hashed_password.c_str(), doubleHashedPassword);

    auto query = transaction{ gameServer_.GetDbConnection() };
    const auto result = query.exec("SELECT * FROM auth.accounts WHERE user_id=$1;", params{ id });

    gameServer_.GetStcProxy().LoginResponse(context, false, "하하하하");
}

void CtsStub::OnRegisterRequest(uint64_t context,
                                   const std::string& id,
                                   const std::string& hashed_password)
{
    char doubleHashedPassword[129];
    Sha512(hashed_password.c_str(), doubleHashedPassword);

    auto query = transaction{ gameServer_.GetDbConnection() };
    const auto result = query.exec("INSERT INTO auth.accounts(user_id, double_hashed_password) VALUES($1, $2) ON CONFLICT (user_id) DO NOTHING;", params{ id, doubleHashedPassword });
    query.commit();

    if (result.affected_rows() > 0)
    {
        gameServer_.GetStcProxy().RegisterResponse(context, RatkiniaProtocol::RegisterResponse_FailedReason_Success);
    }
    else
    {
        gameServer_.GetStcProxy().RegisterResponse(context, RatkiniaProtocol::RegisterResponse_FailedReason::RegisterResponse_FailedReason_ExistingUserId);
    }
}

void Sha512(const char* const inText, char* const outHashed129)
{
    static const char* hexChars = "0123456789abcdef";

    unsigned char hash[64 + 1];

    EVP_MD_CTX* Ctx = EVP_MD_CTX_new();

    ERR_FAIL_COND(0 == EVP_DigestInit_ex(Ctx, EVP_sha512(), nullptr));
    ERR_FAIL_COND(0 == EVP_DigestUpdate(Ctx, inText, 64));
    ERR_FAIL_COND(0 == EVP_DigestFinal_ex(Ctx, hash, nullptr));
    hash[64] = '\0';

    EVP_MD_CTX_free(Ctx);

    for (size_t i = 0; i < 64; ++i)
    {
        outHashed129[i * 2] = hexChars[(hash[i] >> 4) & 0x0F];
        outHashed129[i * 2 + 1] = hexChars[hash[i] & 0x0F];
    }
    outHashed129[128] = '\0';
}