//
// Created by floweryclover on 2025-08-13.
//

#ifndef G_PLAYERCHARACTERS_H
#define G_PLAYERCHARACTERS_H

#include "GlobalObject.h"
#include <unordered_map>

struct G_PlayerCharacters final : GlobalObject
{
    GLOBALOBJECT(G_PlayerCharacters)

    enum class CharacterLinkResult : uint8_t
    {
        Success,
        DuplicateContext,
        CharacterAlreadyActive,
    };

    CharacterLinkResult TryLink(uint32_t context, uint32_t character);

    void UnlinkByContext(uint32_t context);

    void UnlinkByCharacter(uint32_t character);

private:
    std::unordered_map<uint32_t, uint32_t> contextCharacterMap_;
    std::unordered_map<uint32_t, uint32_t> characterContextMap_;
};

#endif //G_PLAYERCHARACTERS_H
