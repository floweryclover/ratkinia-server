//
// Created by floweryclover on 2025-08-27.
//

#include "EventRegistrar.h"

#include "Event_SessionErased.h"
#include "Registrar.h"

void RegisterEvents(Registrar& registrar)
{
    registrar.RegisterEvent<Event_SessionErased>();
}

