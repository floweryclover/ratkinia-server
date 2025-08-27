//
// Created by floweryclover on 2025-08-27.
//

#include "G_Possession.h"

G_Possession::PosessionResult G_Possession::TryPossess(const uint32_t context, const Entity entity)
{
    if (contextEntityMap_.Get(context))
    {
        return PosessionResult::DuplicateContext;
    }
    if (entityContextMap_.Get(entity.GetId()))
    {
        return PosessionResult::EntityAlreadyPossessed;
    }

    contextEntityMap_.EmplaceAt(context, entity);
    entityContextMap_.EmplaceAt(entity.GetId(), entity.GetVersion(), context);
    return PosessionResult::Success;
}

void G_Possession::UnpossessByContext(const uint32_t context)
{
    const auto entity = contextEntityMap_.Get(context);
    if (!entity)
    {
        return;
    }

    entityContextMap_.EraseBySparseIndex(entity->GetId());
    contextEntityMap_.EraseBySparseIndex(context);
}

void G_Possession::UnpossessByEntity(const Entity entity)
{
    const auto versionContext = entityContextMap_.Get(entity.GetId());
    if (!versionContext || versionContext->first != entity.GetVersion())
    {
        return;
    }

    contextEntityMap_.EraseBySparseIndex(versionContext->second);
    entityContextMap_.EraseBySparseIndex(entity.GetId());
}