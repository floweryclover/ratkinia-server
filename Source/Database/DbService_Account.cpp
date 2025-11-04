//
// Created by floweryclover on 2025-11-04.
//

#include "DbService_Account.h"
#include "ErrorMacros.h"
#include <pqxx/pqxx>

using namespace pqxx;

std::optional<std::pair<uint64_t, std::string>> DbService_Account::GetAccountIdPasswordPair(const std::string& account)
{
    nontransaction work{ Connection };
    const auto result = work.exec("SELECT id, password FROM player.accounts WHERE account = $1", params{ account });
    if (result.size() == 0)
    {
        return std::nullopt;
    }
    return std::make_pair(result[0][0].as<uint64_t>(), result[0][1].as<std::string>());
}

DbService_Account::CreateAccountResult DbService_Account::TryCreateAccount(
    const std::string& account,
    const std::string& password)
{

    if (nontransaction checkIfExistsWork{ Connection };
        checkIfExistsWork.query_value<bool>("SELECT EXISTS(SELECT 1 FROM player.accounts WHERE account = $1)", params{account}))
    {
        return CreateAccountResult::DuplicateId;
    }

    try
    {
        work insertWork{ Connection };
        const auto result = insertWork.exec("INSERT INTO player.accounts (account, password) VALUES ($1, $2)", params{account, password});
        insertWork.commit();
        return result.affected_rows() > 0 ? CreateAccountResult::Success : CreateAccountResult::UnknownError;
    }
    catch (const std::exception& e)
    {
        CRASH_NOW_MSG(e.what());
    }
}
