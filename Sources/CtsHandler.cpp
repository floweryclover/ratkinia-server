//
// Created by floweryclover on 2025-05-05.
//

#include "CtsHandler.h"
#include "GameServer.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <pqxx/pqxx>
#include <iostream>

CtsHandler::CtsHandler(GameServer& gameServer)
    : gameServer_{ gameServer }
{

}

void CtsHandler::OnParseMessageFailed(uint64_t context,
                                      RatkiniaProtocol::CtsMessageType messageType)
{

}

void CtsHandler::OnUnknownMessageType(uint64_t context,
                                      RatkiniaProtocol::CtsMessageType messageType)
{

}

void CtsHandler::OnLoginRequest(uint64_t context,
                                const std::string& id,
                                const std::string& hashed_password)
{

    std::cout << context << ": " << id << ", " << hashed_password << std::endl;
//    constexpr const char* HexChars = "0123456789abcdef";
//
//    unsigned char HashedPassword[64 + 1];
//    const char* PasswordString = "안녕";
//
//    EVP_MD_CTX* Ctx = EVP_MD_CTX_new();
//
//
//    check(1 == EVP_DigestInit_ex(Ctx, EVP_sha512(), nullptr));
//
//    check(1 == EVP_DigestUpdate(Ctx, PasswordString, 64));
//
//    check(1 == EVP_DigestFinal_ex(Ctx, HashedPassword, nullptr))
//    HashedPassword[64] = '\0';
//
//    EVP_MD_CTX_free(Ctx);
//
//    char HexString[129];
//    for (size_t i = 0; i < 64; ++i)
//    {
//        HexString[i * 2] = HexChars[(HashedPassword[i] >> 4) & 0x0F];
//        HexString[i * 2 + 1] = HexChars[HashedPassword[i] & 0x0F];
//    }
//    HexString[128] = '\0';
//    RatkiniaClient->LoginRequest(0,
//                                 "asdasdasdas", HexString);
}

void CtsHandler::OnRegisterRequest(uint64_t context,
                                   const std::string& id,
                                   const std::string& hashed_password)
{

}
