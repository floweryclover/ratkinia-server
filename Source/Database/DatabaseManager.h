//
// Created by floweryclover on 2025-08-04.
//

#ifndef DATABASE_H
#define DATABASE_H

#include "Manager.h"
#include <memory>
#include <vector>

class Database;

namespace pqxx
{
    class connection;
}

class DatabaseManager final : public Manager
{
public:
    explicit DatabaseManager(const char* options);

    ~DatabaseManager() override;

    template<typename TDatabaseTable>
    void Register()
    {
        TDatabaseTable::SetRuntimeOrder(databaseTables_.size());
        databaseTables_.push_back(std::make_unique<TDatabaseTable>(*connection_));
    }

    template<typename TDatabaseTable>
    const TDatabaseTable& Get() const
    {
        return static_cast<const TDatabaseTable&>(*databaseTables_[TDatabaseTable::GetRuntimeOrder()]);
    }

    template<typename TDatabaseTable>
    TDatabaseTable& Get()
    {
        return const_cast<TDatabaseTable&>(static_cast<const DatabaseManager*>(this)->Get<TDatabaseTable>());
    }

private:
    std::vector<std::unique_ptr<Database>> databaseTables_;
    std::unique_ptr<pqxx::connection> connection_;
};

#endif //DATABASE_H
