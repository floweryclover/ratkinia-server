//
// Created by floweryclover on 2025-09-30.
//

#include "D_PlayerCharacters.h"
#include <pqxx/pqxx>
#include <regex>

using namespace pqxx;

D_PlayerCharacters::D_PlayerCharacters(connection& connection)
    : Database{ connection }
{
    nontransaction selectPlayerCharactersWork{ dbConnection };

    for (const auto playerCharacters = selectPlayerCharactersWork.exec("SELECT * FROM player.characters");
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

std::pair<D_PlayerCharacters::CreateCharacterResult, uint64_t> D_PlayerCharacters::TryCreatePlayerCharacter(
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
        work insertCharacterWork{ dbConnection };
        const auto result = insertCharacterWork.exec(
            "INSERT INTO player.characters (player_id, name) VALUES ($1, $2) RETURNING id",
            params{ playerId, name });
        insertCharacterWork.commit();
        id = result[0][0].as<uint64_t>();
    } catch (const integrity_constraint_violation& e)
    {
        std::osyncstream{std::cout} << e.what() << std::endl;
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
