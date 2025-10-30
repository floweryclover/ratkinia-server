//
// Created by floweryclover on 2025-09-29.
//

#ifndef TABLE_H
#define TABLE_H

#include "ErrorMacros.h"
#include "RuntimeOrder.h"
#include <cstdint>

namespace pqxx
{
    class connection;
}

class Table
{
public:
    explicit Table(pqxx::connection& connection)
        : dbConnection{ connection }
    {
    }

    virtual ~Table() = default;

    Table(const Table&) = delete;

    Table& operator=(const Table&) = delete;

    Table(Table&&) = delete;

    Table& operator=(Table&&) = delete;

protected:
    pqxx::connection& dbConnection;
};

#define TABLE(TClass)                                                                  \
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

#endif //TABLE_H
