//
// Created by floweryclover on 2025-08-13.
//

#include "G_PlayerCharacters.h"

G_PlayerCharacters::CharacterLinkResult G_PlayerCharacters::TryLink(const uint32_t context, const uint32_t character)
{
    if (contextCharacterMap_.contains(context))
    {
        return CharacterLinkResult::DuplicateContext;
    }
    if (characterContextMap_.contains(character))
    {
        return CharacterLinkResult::CharacterAlreadyActive;
    }

    contextCharacterMap_.emplace(context, character);
    characterContextMap_.emplace(character, context);
    return CharacterLinkResult::Success;
}

void G_PlayerCharacters::UnlinkByContext(const uint32_t context)
{
    const auto contextCharacter = contextCharacterMap_.find(context);
    if (contextCharacter == contextCharacterMap_.end())
    {
        return;
    }

    characterContextMap_.erase(contextCharacter->second);
    contextCharacterMap_.erase(contextCharacter);
}

void G_PlayerCharacters::UnlinkByCharacter(const uint32_t character)
{
    const auto characterContext = characterContextMap_.find(character);
    if (characterContext == characterContextMap_.end())
    {
        return;
    }

    contextCharacterMap_.erase(characterContext->second);
    characterContextMap_.erase(characterContext);
}