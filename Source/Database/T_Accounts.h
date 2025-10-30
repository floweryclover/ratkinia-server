//
// Created by floweryclover on 2025-09-29.
//

#ifndef D_ACCOUNTS_H
#define D_ACCOUNTS_H

#include "Table.h"
#include <absl/container/flat_hash_map.h>

class T_Accounts final : public Table
{
    TABLE(T_Accounts)

public:
    enum class CreateAccountResult
    {
        Success,
        DuplicateId,
        UnknownError,
    };

    struct Row
    {
        uint64_t Id;
        std::string Account; // 6~24자, [a-z0-9_]
        std::string Password; // 8~32자, 알파벳대소, 특수문자. [a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]
    };

    CreateAccountResult TryCreateAccount(const std::string& account, const std::string& password);

    const Row* TryGetAccountByUserId(const std::string& userId) const
    {
        return accountsByUserId_.contains(userId) ? accountsByUserId_.at(userId) : nullptr;
    }

private:
    absl::flat_hash_map<uint64_t, std::unique_ptr<Row>> accounts_;
    absl::flat_hash_map<std::string, Row*> accountsByUserId_;
};

#endif //D_ACCOUNTS_H
