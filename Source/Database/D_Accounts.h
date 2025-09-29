//
// Created by floweryclover on 2025-09-29.
//

#ifndef D_ACCOUNTS_H
#define D_ACCOUNTS_H

#include "Database.h"
#include <unordered_map>

class D_Accounts final : public Database
{
    DATABASE(D_Accounts)

public:
    enum class CreateAccountResult
    {
        Success,
        DuplicateId,
        UnknownError,
    };

    struct AccountRow
    {
        uint64_t Id;
        std::string Account; // 6~24자, [a-z0-9_]
        std::string Password; // 8~32자, 알파벳대소, 특수문자. [a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]
    };

    CreateAccountResult TryCreateAccount(const std::string& account, const std::string& password);

    const AccountRow* TryGetAccountByUserId(const std::string& userId) const
    {
        return accountsByUserId_.contains(userId) ? accountsByUserId_.at(userId) : nullptr;
    }

private:
    std::unordered_map<uint64_t, AccountRow> accounts_;
    std::unordered_map<std::string, const AccountRow*> accountsByUserId_;
};

#endif //D_ACCOUNTS_H
