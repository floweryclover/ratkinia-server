//
// Created by floweryclover on 2025-10-30.
//

#include "DatabaseRegistrar.h"
#include "DatabaseServer.h"
#include "T_Accounts.h"
#include "T_PlayerCharacters.h"

void RegisterDatabase(DatabaseServer& databaseServer)
{
    databaseServer.Register<T_Accounts>();
    databaseServer.Register<T_PlayerCharacters>();
}

