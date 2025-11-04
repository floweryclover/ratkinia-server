//
// Created by floweryclover on 2025-11-04.
//

#ifndef DBSERVICE_H
#define DBSERVICE_H

#include <functional>

namespace pqxx
{
    class connection;
}

struct DbServiceInitializer final
{
    std::reference_wrapper<pqxx::connection> DbConnection;
};

class DbService
{
public:
    virtual ~DbService() = 0;

    DbService(const DbService& dbService) = delete;

    DbService& operator=(const DbService& dbService) = delete;

    DbService(DbService&& dbService) = delete;

    DbService& operator=(DbService&& dbService) = delete;

protected:
    pqxx::connection& Connection;

    explicit DbService(const DbServiceInitializer& initializer)
        : Connection{ initializer.DbConnection.get() }
    {
    }
};

inline DbService::~DbService() = default;

#endif //DBSERVICE_H
