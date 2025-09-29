//
// Created by floweryclover on 2025-09-30.
//

#include "DatabaseRegistrar.h"
#include "DatabaseManager.h"

#include "D_Accounts.h"
#include "D_Components.h"
#include "D_PlayerCharacters.h"

void RegisterDatabases(DatabaseManager& databaseManager)
{
    databaseManager.Register<D_Accounts>();
    databaseManager.Register<D_Components>();
    databaseManager.Register<D_PlayerCharacters>();
}
