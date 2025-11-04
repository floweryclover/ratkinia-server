//
// Created by floweryclover on 2025-11-04.
//

#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include "DbService.h"
#include <memory>

namespace pqxx
{
    class connection;
}

class DbConnectionPool;

class DbConnection final
{
public:
    explicit DbConnection(std::unique_ptr<pqxx::connection> pqxxConnection, DbConnectionPool& ownerPool);

    ~DbConnection();

    DbConnection(const DbConnection&) = delete;

    DbConnection& operator=(const DbConnection&) = delete;

    DbConnection(DbConnection&&) = delete;

    DbConnection& operator=(DbConnection&&) = delete;

    template<typename TDbService>
    TDbService Access()
    {
        return TDbService{ DbServiceInitializer{ *pqxxConnection_ } };
    }

private:
    std::unique_ptr<pqxx::connection> pqxxConnection_;
    DbConnectionPool& ownerPool_;
};

#endif //DBCONNECTION_H
