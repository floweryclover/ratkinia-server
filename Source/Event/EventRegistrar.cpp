//
// Created by floweryclover on 2025-08-27.
//

#include "EventRegistrar.h"
#include "EventManager.h"

#include "Event_SessionErased.h"

void RegisterEvents(EventManager& eventManager)
{
    eventManager.Register<Event_SessionErased>();
}

