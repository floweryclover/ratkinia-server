//
// Created by floweryclover on 2025-08-27.
//

#include "S_Initializer_PlayerCharacters.h"
#include "Environment.h"
#include "../../GlobalObject/GlobalObjectManager.h"
#include "G_PlayerCharacters.h"
#include <pqxx/pqxx>

#include "../../Component/ComponentManager.h"
#include "C_HumanLikeBody.h"
#include "D_PlayerCharacters.h"
#include "../../Database/DatabaseManager.h"
#include "EntityManager.h"

using namespace pqxx;

void S_Initializer_PlayerCharacters(const MutableEnvironment& environment)
{
    auto& g_playerCharacters = environment.GlobalObjectManager.Get<G_PlayerCharacters>();
    auto& d_playerCharacters = environment.DatabaseManager.Get<D_PlayerCharacters>();

    for (const auto [characterId, row] : d_playerCharacters.PlayerCharacters())
    {
        const uint32_t playerId = row.PlayerId;
        const auto entity = environment.EntityManager.Create();
        g_playerCharacters.AddOwnership(playerId, characterId, entity);
        environment.ComponentManager.AttachComponentTo<C_HumanLikeBody>(entity);
    }
}
