//
// Created by floweryclover on 2025-09-28.
//

#include "DatabaseManager.h"
#include "MessagePrinter.h"
#include <pqxx/pqxx>
#include <regex>


using namespace pqxx;

DatabaseManager::DatabaseManager(const char* const options)
    : connection_{ std::make_unique<connection>(options) }
{
    nontransaction work{ *connection_ };
    for (const auto accounts = work.exec("SELECT * FROM player.accounts");
         const auto row : accounts)
    {
        const auto [pair, emplaced] = accounts_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(row[0].as<uint64_t>()),
            std::forward_as_tuple(row[0].as<uint64_t>(), row[1].as<std::string>(), row[2].as<std::string>()));

        accountsByUserId_.emplace(pair->second.Account, &pair->second);
    }

    for (const auto playerCharacters = work.exec("SELECT * FROM player.characters");
         const auto row : playerCharacters)
    {
        const auto [pair, emplaced] = playerCharacters_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(row[0].as<int64_t>()),
            std::forward_as_tuple(row[0].as<int64_t>(), row[1].as<uint64_t>(), row[2].as<std::string>()));

        playerCharactersByPlayerId_[pair->second.PlayerId].push_back(&pair->second);
        playerCharactersByName_[pair->second.Name] = &pair->second;
    }
}

DatabaseManager::~DatabaseManager() = default;

DatabaseManager::CreateAccountResult DatabaseManager::TryCreateAccount(const std::string& account,
                                                                       const std::string& password)
{
    if (accountsByUserId_.contains(account))
    {
        return CreateAccountResult::DuplicateId;
    }

    uint64_t id;
    try
    {
        work insertAccountWork{ *connection_ };
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

std::pair<DatabaseManager::CreateCharacterResult, uint64_t> DatabaseManager::TryCreatePlayerCharacter(
    const uint64_t playerId,
    std::string name)
{
    if (playerCharactersByName_.contains(name))
    {
        return { CreateCharacterResult::DuplicateName, 0 };
    }

    std::regex invalidNameRegex{ R"([^a-z0-9가-힇])" };
    if (name.length() < 2 || name.length() > 16 ||
        std::regex_search(name, invalidNameRegex))
    {
        return { CreateCharacterResult::InvalidFormat, 0 };
    }

    uint64_t id;
    try
    {
        work insertCharacterWork{ *connection_ };
        const auto result = insertCharacterWork.exec(
            "INSERT INTO player.characters (player_id, name) VALUES ($1, $2) RETURNING id",
            params{ playerId, name });
        insertCharacterWork.commit();
        id = result[0][0].as<uint64_t>();
    } catch (const integrity_constraint_violation& e)
    {
        MessagePrinter::WriteErrorLine(e.what());
        return { CreateCharacterResult::UnknownError, 0 };
    }

    const auto [pair, emplaced] = playerCharacters_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(id, playerId, name));

    playerCharactersByPlayerId_[playerId].push_back(&pair->second);
    playerCharactersByName_[std::move(name)] = &pair->second;

    return { CreateCharacterResult::Success, 0 };
}
