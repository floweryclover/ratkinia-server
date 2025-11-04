//
// Created by floweryclover on 2025-11-05.
//

#include "DbConnection.h"
#include "DbConnectionPool.h"
#include <pqxx/pqxx>

DbConnection::DbConnection(std::unique_ptr<pqxx::connection> pqxxConnection, DbConnectionPool& ownerPool)
    : pqxxConnection_{ std::move(pqxxConnection) },
      ownerPool_{ ownerPool }
{
}

DbConnection::~DbConnection()
{
    ownerPool_.Release(std::move(pqxxConnection_));
}
