//
// Created by floweryclover on 2025-09-29.
//

#include "D_Accounts.h"
#include <pqxx/pqxx>

using namespace pqxx;

D_Accounts::D_Accounts(connection& connection)
    : Database{ connection }
{
    nontransaction selectAccountsWork{ dbConnection };
    for (const auto accounts = selectAccountsWork.exec("SELECT * FROM player.accounts");
         const auto row : accounts)
    {
        const auto [pair, emplaced] = accounts_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(row[0].as<uint64_t>()),
            std::forward_as_tuple(row[0].as<uint64_t>(), row[1].as<std::string>(), row[2].as<std::string>()));

        accountsByUserId_.emplace(pair->second.Account, &pair->second);
    }
}

D_Accounts::CreateAccountResult D_Accounts::TryCreateAccount(const std::string& account,
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
        MessagePrinter::WriteErrorLine(e.what());
        return CreateAccountResult::UnknownError;
    }

    const auto [pair, emplaced] = accounts_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(id, account, std::move(password)));
    accountsByUserId_.emplace(std::move(account), &pair->second);

    return CreateAccountResult::Success;
}
