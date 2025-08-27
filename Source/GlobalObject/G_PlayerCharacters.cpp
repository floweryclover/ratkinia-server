//
// Created by floweryclover on 2025-08-13.
//

#include "G_PlayerCharacters.h"
#include <format>

void G_PlayerCharacters::AddOwnership(const uint32_t playerId, const uint32_t characterId, const Entity entity)
{
    const bool integrityError = characterEntityMap_.Get(characterId) ||
                                entityPlayerMap_.Get(entity.GetId()) ||
                                entityCharacterMap_.Get(entity.GetId());
    CRASH_COND_MSG(integrityError,
                      std::format("PlayerId: {}, CharacterId: {}, Entity: {}", playerId, characterId, entity.GetId()));

    characterEntityMap_.EmplaceAt(characterId, entity);
    entityPlayerMap_.EmplaceAt(entity.GetId(), entity.GetVersion(), playerId);
    entityCharacterMap_.EmplaceAt(entity.GetId(), entity.GetVersion(), characterId);
}

Entity G_PlayerCharacters::GetEntityOf(const uint32_t playerId, const uint32_t characterId)
{
    const auto entity = characterEntityMap_.Get(characterId);
    if (!entity)
    {
        return Entity::NullEntity();
    }

    const auto entityCharacter = entityCharacterMap_.Get(entity->GetId());
    const auto entityPlayer = entityPlayerMap_.Get(entity->GetId());
    const bool integrityError = !entityCharacter || !entityPlayer;
    CRASH_COND_MSG(integrityError, std::format("PlayerId: {}, CharacterId: {}, Entity: {}", playerId, characterId, entity->GetId()));

    if (entityPlayer->second != playerId)
    {
        return Entity::NullEntity();
    }
    return *entity;
}
