//
// Created by floweryclover on 2025-09-28.
//

#include "DatabaseManager.h"
#include "Database.h"
#include <pqxx/pqxx>

using namespace pqxx;

DatabaseManager::DatabaseManager(const char* const options)
    : connection_{ std::make_unique<connection>(options) }
{
}

DatabaseManager::~DatabaseManager() = default;