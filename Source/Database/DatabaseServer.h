//
// Created by floweryclover on 2025-10-30.
//

#ifndef DATABASE_H
#define DATABASE_H

#include "ErrorMacros.h"
#include <vector>
#include <memory>

namespace pqxx
{
    class connection;
}

class Table;

class DatabaseServer final
{
public:
    explicit DatabaseServer(const char* dbHost);

    ~DatabaseServer();

    DatabaseServer(const DatabaseServer&) = delete;

    DatabaseServer& operator=(const DatabaseServer&) = delete;

    DatabaseServer(DatabaseServer&&) = delete;

    DatabaseServer& operator=(DatabaseServer&&) = delete;

    void Finalize()
    {
        CRASH_COND(isFinalized_);
        isFinalized_ = true;
    }

    template<typename TTable>
    void Register()
    {
        CRASH_COND(isFinalized_);
        TTable::SetRuntimeOrder(tables_.size());
        tables_.push_back(std::make_unique<TTable>(*connection_));
    }

    template<typename TTable>
    TTable& Get()
    {
        return static_cast<TTable&>(*tables_[TTable::GetRuntimeOrder()]);
    }

private:
    bool isFinalized_;
    std::vector<std::unique_ptr<Table>> tables_;
    std::unique_ptr<pqxx::connection> connection_;
};

#endif //DATABASE_H
