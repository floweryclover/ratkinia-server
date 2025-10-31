//
// Created by floweryclover on 2025-10-30.
//

#include "DatabaseServer.h"
#include "Table.h"
#include <pqxx/pqxx>

using namespace pqxx;

DatabaseServer::DatabaseServer(const char* const dbHost)
    : isFinalized_{ false },
      connection_{ std::make_unique<connection>(dbHost) }
{
}

DatabaseServer::~DatabaseServer() = default;
