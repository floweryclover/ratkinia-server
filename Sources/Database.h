//
// Created by floweryclover on 2025-08-04.
//

#ifndef DATABASE_H
#define DATABASE_H

#include <pqxx/prepared_statement>

namespace Database
{
    inline static pqxx::prepped Prepped_FindUserId{ "find_user_id" };
    inline static pqxx::prepped Prepped_InsertAccount{ "insert_account" };
}

#endif //DATABASE_H
