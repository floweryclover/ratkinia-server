//
// Created by floweryclover on 2025-11-04.
//

#ifndef DBSERVICE_ACCOUNT_H
#define DBSERVICE_ACCOUNT_H

#include "DbService.h"
#include <optional>
#include <string>

class DbService_Account final : public DbService
{
public:
    enum class CreateAccountResult
    {
        Success,
        DuplicateId,
        UnknownError,
    };

    explicit DbService_Account(const DbServiceInitializer& initializer)
        : DbService{ initializer }
    {
    }

    [[nodiscard]]
    std::optional<std::pair<uint64_t, std::string>> GetAccountIdPasswordPair(const std::string& account);

    [[nodiscard]]
    CreateAccountResult TryCreateAccount(const std::string& account, const std::string& password);
};

#endif //DBSERVICE_ACCOUNT_H
