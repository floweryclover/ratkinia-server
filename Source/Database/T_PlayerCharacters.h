//
// Created by floweryclover on 2025-09-30.
//

#ifndef D_PLAYERCHARACTERS_H
#define D_PLAYERCHARACTERS_H

#include "Table.h"
#include <absl/container/flat_hash_map.h>

class T_PlayerCharacters final : public Table
{
    TABLE(T_PlayerCharacters)

public:
    enum class CreateCharacterResult
    {
        Success,
        DuplicateName,
        InvalidFormat,
        UnknownError,
    };

    struct PlayerCharacterRow
    {
        uint64_t Id;
        uint64_t PlayerId;
        std::string Name; // 2~16자, [a-zA-Z0-9가-힣]
    };

    // /**
    //  *
    //  * @param playerId
    //  * @param name
    //  * @return Result가 success일 경우 second에는 생성된 Character Id.
    //  */
    // std::pair<CreateCharacterResult, uint64_t> TryCreatePlayerCharacter(uint64_t playerId, std::string name);
    //
    // const std::vector<const PlayerCharacterRow*>* TryGetPlayerCharactersByPlayerId(const uint64_t playerId) const
    // {
    //     return playerCharactersByPlayerId_.contains(playerId) ? &playerCharactersByPlayerId_.at(playerId) : nullptr;
    // }

private:
    absl::flat_hash_map<uint64_t, PlayerCharacterRow> playerCharacters_;
    absl::flat_hash_map<uint64_t, std::vector<uint64_t>> playerCharactersByPlayerId_;
    absl::flat_hash_map<std::string, uint64_t> playerCharactersByName_;
};

#endif //D_PLAYERCHARACTERS_H
