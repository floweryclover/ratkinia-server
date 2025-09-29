//
// Created by floweryclover on 2025-09-29.
//

#ifndef DATABASETABLE_H
#define DATABASETABLE_H

#include "ErrorMacros.h"
#include "RuntimeOrder.h"
#include <cstdint>

namespace pqxx
{
    class connection;
}

class Database
{
public:
    explicit Database(pqxx::connection& connection)
        : dbConnection{ connection }
    {
    }

    virtual ~Database() = default;

    Database(const Database&) = delete;

    Database& operator=(const Database&) = delete;

    Database(Database&&) = delete;

    Database& operator=(Database&&) = delete;

protected:
    pqxx::connection& dbConnection;
};

#define DATABASE(TClass)                                                                  \
public:                                                                                         \
    explicit TClass(pqxx::connection& connection);                                              \
                                                                                                \
    static void SetRuntimeOrder(const uint32_t runtimeOrder)                                    \
    {                                                                                           \
        CRASH_COND(RuntimeOrder != UnregisteredRuntimeOrder);                                   \
        RuntimeOrder = runtimeOrder;                                                            \
    }                                                                                           \
                                                                                                \
    static uint32_t GetRuntimeOrder()                                                           \
    {                                                                                           \
        CRASH_COND(RuntimeOrder == UnregisteredRuntimeOrder);                                   \
        return RuntimeOrder;                                                                    \
    }                                                                                           \
                                                                                                \
private:                                                                                        \
    inline static uint32_t RuntimeOrder = UnregisteredRuntimeOrder;

#endif //DATABASETABLE_H
