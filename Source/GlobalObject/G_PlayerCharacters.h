//
// Created by floweryclover on 2025-08-13.
//

#ifndef G_PLAYERCHARACTERS_H
#define G_PLAYERCHARACTERS_H

#include "Entity.h"
#include "GlobalObject.h"
#include "MemoryPool/SparseList.h"

/**
 * 플레이어 소유의 캐릭터들의 정보를 관리하는 글로벌 오브젝트.
 */
struct G_PlayerCharacters final : GlobalObject
{
    GLOBALOBJECT(G_PlayerCharacters)

    void AddOwnership(uint32_t playerId, uint32_t characterId, Entity entity);

    Entity GetEntityOf(uint32_t playerId, uint32_t characterId);

private:
    SparseList<Entity> characterEntityMap_;
    SparseList<std::pair<uint32_t, uint32_t>> entityPlayerMap_; // [Version, PlayerId]
    SparseList<std::pair<uint32_t, uint32_t>> entityCharacterMap_; // [Version, CharacterId]
};

#endif //G_PLAYERCHARACTERS_H
