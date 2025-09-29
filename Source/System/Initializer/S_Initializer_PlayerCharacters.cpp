//
// Created by floweryclover on 2025-08-27.
//

#include "S_Initializer_PlayerCharacters.h"
#include "Environment.h"
#include "GlobalObjectManager.h"
#include "G_PlayerCharacters.h"
#include <pqxx/pqxx>

#include "ComponentManager.h"
#include "C_HumanLikeBody.h"
#include "DatabaseManager.h"
#include "EntityManager.h"

using namespace pqxx;

void S_Initializer_PlayerCharacters(const MutableEnvironment& environment)
{
    auto& g_playerCharacters = environment.GlobalObjectManager.Get<G_PlayerCharacters>();

    for (const auto [characterId, row] : environment.DatabaseManager.PlayerCharacters())
    {
        const uint32_t playerId = row.PlayerId;
        const auto entity = environment.EntityManager.Create();
        g_playerCharacters.AddOwnership(playerId, characterId, entity);
        environment.ComponentManager.AttachComponentTo<C_HumanLikeBody>(entity);
    }
}
