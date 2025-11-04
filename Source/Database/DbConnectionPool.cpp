//
// Created by floweryclover on 2025-11-04.
//

#include "DbConnectionPool.h"
#include "ErrorMacros.h"
#include <pqxx/pqxx>

using namespace pqxx;

DbConnectionPool::DbConnectionPool(const char* const dbHost, const size_t maxConnections)
{
    try
    {
        for (int i = 0; i < maxConnections; ++i)
        {
            connections_.emplace_back(std::make_unique<connection>(dbHost));
        }
    } catch (const broken_connection& e)
    {
        CRASH_NOW_MSG(e.what());
    }
}

DbConnectionPool::~DbConnectionPool() = default;

DbConnection DbConnectionPool::Acquire()
{
    auto pqxxConnection =
        [&]
        {
            std::unique_lock lock{ mutex_ };
            conditionVariable_.wait(lock,
                                    [&]
                                    {
                                        return !connections_.empty();
                                    });
            auto connection = std::move(connections_.back());
            connections_.pop_back();
            return std::move(connection);
        }();

    return DbConnection{ std::move(pqxxConnection), *this };
}

void DbConnectionPool::Release(std::unique_ptr<connection> connection)
{
    std::unique_lock lock{ mutex_ };
    connections_.emplace_back(std::move(connection));
    conditionVariable_.notify_one();
}
