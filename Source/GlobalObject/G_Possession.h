//
// Created by floweryclover on 2025-08-27.
//

#ifndef G_POSSESSION_H
#define G_POSSESSION_H

#include "GlobalObject.h"
#include "Entity.h"
#include "MemoryPool/SparseList.h"

/**
 * 플레이어가 빙의 중인 엔티티 정보를 저장하는 글로벌 오브젝트.
 */
struct G_Possession final : GlobalObject
{
    GLOBALOBJECT(G_PlayerCharacters)

    enum class PosessionResult : uint8_t
    {
        Success,
        DuplicateContext,
        EntityAlreadyPossessed,
    };

    PosessionResult TryPossess(uint32_t context, Entity entity);

    void UnpossessByContext(uint32_t context);

    void UnpossessByEntity(Entity entity);

private:
    SparseList<Entity> contextEntityMap_;
    SparseList<std::pair<uint32_t, uint32_t>> entityContextMap_; // [Version, Context]
};

#endif //G_POSSESSION_H
