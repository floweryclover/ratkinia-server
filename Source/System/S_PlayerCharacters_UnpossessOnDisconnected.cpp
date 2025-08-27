//
// Created by floweryclover on 2025-08-20.
//

#include "S_PlayerCharacters_UnpossessOnDisconnected.h"

#include "Environment.h"
#include "EventManager.h"
#include "GlobalObjectManager.h"

#include "Event_SessionErased.h"
#include "G_Possession.h"

void S_PlayerCharacters_UnpossessOnDisconnected(const MutableEnvironment& environment)
{
    for (const auto& event : environment.EventManager.Events<Event_SessionErased>())
    {
        environment.GlobalObjectManager.Get<G_Possession>().UnpossessByContext(event.Context);
    }
}
