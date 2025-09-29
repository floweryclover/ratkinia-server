//
// Created by floweryclover on 2025-09-30.
//

#ifndef D_PLAYERCHARACTERS_H
#define D_PLAYERCHARACTERS_H

#include "Database.h"
#include <unordered_map>

class D_PlayerCharacters final : public Database
{
    DATABASE(D_PlayerCharacters)

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

    /**
     *
     * @param playerId
     * @param name
     * @return Result가 success일 경우 second에는 생성된 Character Id.
     */
    std::pair<CreateCharacterResult, uint64_t> TryCreatePlayerCharacter(uint64_t playerId, std::string name);

    const std::vector<const PlayerCharacterRow*>* TryGetPlayerCharactersByPlayerId(const uint64_t playerId) const
    {
        return playerCharactersByPlayerId_.contains(playerId) ? &playerCharactersByPlayerId_.at(playerId) : nullptr;
    }

    const std::unordered_map<uint64_t, PlayerCharacterRow>& PlayerCharacters() const
    {
        return playerCharacters_;
    }
private:
    std::unordered_map<uint64_t, PlayerCharacterRow> playerCharacters_;
    std::unordered_map<uint64_t, std::vector<const PlayerCharacterRow*>> playerCharactersByPlayerId_;
    std::unordered_map<std::string, const PlayerCharacterRow*> playerCharactersByName_;
};

#endif //D_PLAYERCHARACTERS_H
