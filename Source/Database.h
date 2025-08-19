//
// Created by floweryclover on 2025-08-04.
//

#ifndef DATABASE_H
#define DATABASE_H

#include <pqxx/prepared_statement>

namespace Database
{
    inline static pqxx::prepped Prepped_FindUserId{ "find_user_id" };
    inline static pqxx::prepped Prepped_CreateAccount{ "create_account" };
    inline static pqxx::prepped Prepped_FindPlayerCharacterByName{ "find_player_character_by_name" };
    inline static pqxx::prepped Prepped_CreatePlayerCharacter{ "create_player_character" };
    inline static pqxx::prepped Prepped_LoadMyCharacters{ "load_my_characters" };
}

#endif //DATABASE_H
