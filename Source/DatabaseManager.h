//
// Created by floweryclover on 2025-08-04.
//

#ifndef DATABASE_H
#define DATABASE_H

#include <unordered_map>
#include <string>
#include <memory>

namespace pqxx
{
    class connection;
}

class DatabaseManager
{
public:
    enum class CreateAccountResult
    {
        Success,
        DuplicateId,
        UnknownError,
    };

    enum class CreateCharacterResult
    {
        Success,
        DuplicateName,
        InvalidFormat,
        UnknownError,
    };

    struct AccountRow
    {
        uint64_t Id;
        std::string Account; // 6~24자, [a-z0-9_]
        std::string Password; // 8~32자, 알파벳대소, 특수문자. [a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]
    };

    struct PlayerCharacterRow
    {
        uint64_t Id;
        uint64_t PlayerId;
        std::string Name; // 2~16자, [a-zA-Z0-9가-힣]
    };

    explicit DatabaseManager(const char* options);

    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;

    DatabaseManager& operator=(const DatabaseManager&) = delete;

    DatabaseManager(DatabaseManager&&) = delete;

    DatabaseManager& operator=(DatabaseManager&&) = delete;

    CreateAccountResult TryCreateAccount(const std::string& account, const std::string& password);

    const AccountRow* TryGetAccountByUserId(const std::string& userId) const
    {
        return accountsByUserId_.contains(userId) ? accountsByUserId_.at(userId) : nullptr;
    }
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
    std::unique_ptr<pqxx::connection> connection_;

    std::unordered_map<uint64_t, AccountRow> accounts_;
    std::unordered_map<std::string, const AccountRow*> accountsByUserId_;

    std::unordered_map<uint64_t, PlayerCharacterRow> playerCharacters_;
    std::unordered_map<uint64_t, std::vector<const PlayerCharacterRow*>> playerCharactersByPlayerId_;
    std::unordered_map<std::string, const PlayerCharacterRow*> playerCharactersByName_;
};

#endif //DATABASE_H
