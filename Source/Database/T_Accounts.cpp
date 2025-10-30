//
// Created by floweryclover on 2025-09-29.
//

#include "T_Accounts.h"
#include <pqxx/pqxx>

using namespace pqxx;

T_Accounts::T_Accounts(connection& connection)
    : Table{ connection }
{
    nontransaction selectAccountsWork{ dbConnection };
    for (const auto accounts = selectAccountsWork.exec("SELECT * FROM player.accounts");
         const auto row : accounts)
    {
        const auto [pair, emplaced] = accounts_.emplace(
            row[0].as<uint64_t>(),
            std::make_unique<Row>(row[0].as<uint64_t>(), row[1].as<std::string>(), row[2].as<std::string>()));

        accountsByUserId_.emplace(pair->second->Account, pair->second.get());
    }
}

T_Accounts::CreateAccountResult T_Accounts::TryCreateAccount(const std::string& account,
                                                             const std::string& password)
{
    if (accountsByUserId_.contains(account))
    {
        return CreateAccountResult::DuplicateId;
    }

    uint64_t id;
    try
    {
        work insertAccountWork{ dbConnection };
        const auto result = insertAccountWork.exec(
            "INSERT INTO player.accounts (account, password) VALUES ($1, $2) RETURNING id",
            params{ account, password });
        insertAccountWork.commit();
        id = result[0][0].as<uint64_t>();
    } catch (const integrity_constraint_violation& e)
    {
        std::osyncstream{std::cout} << e.what() << std::endl;
        return CreateAccountResult::UnknownError;
    }

    const auto [pair, emplaced] = accounts_.emplace(
        id,
        std::make_unique<Row>(id, account, std::move(password)));
    accountsByUserId_.emplace(std::move(account), pair->second.get());

    return CreateAccountResult::Success;
}
