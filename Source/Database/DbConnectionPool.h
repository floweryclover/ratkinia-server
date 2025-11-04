//
// Created by floweryclover on 2025-11-04.
//

#ifndef DBCONNECTIONPOOL_H
#define DBCONNECTIONPOOL_H

#include "DbConnection.h"
#include <mutex>
#include <vector>

class DbConnectionPool final
{
public:
    explicit DbConnectionPool(const char* dbHost, size_t maxConnections);

    ~DbConnectionPool();

    DbConnectionPool(const DbConnectionPool&) = delete;

    DbConnectionPool& operator=(const DbConnectionPool&) = delete;

    DbConnectionPool(DbConnectionPool&&) = delete;

    DbConnectionPool& operator=(DbConnectionPool&&) = delete;

    DbConnection Acquire();

    void Release(std::unique_ptr<pqxx::connection> connection);

private:
    std::mutex mutex_;
    std::condition_variable conditionVariable_;
    std::vector<std::unique_ptr<pqxx::connection>> connections_;
};

#endif //DBCONNECTIONPOOL_H
