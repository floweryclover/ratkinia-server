//
// Created by floweryclover on 2025-10-30.
//

#include "Actor.h"
#include "ErrorMacros.h"

void Actor::Run()
{
    if (messageQueue_[1 - pushIndex_].empty())
    {
        std::scoped_lock lock{ pushMutex_ };
        pushIndex_ = 1 - pushIndex_;
    }

    for (auto& message : messageQueue_[1 - pushIndex_])
    {
        const uint32_t typeIndex = message->TypeIndex;
        const auto handler = messageHandlers_.find(typeIndex);
        if (handler != messageHandlers_.end())
        {
            (*handler->second)(*this, std::move(message));
        }
        else
        {
            OnUnknownMessageReceived(std::move(message));
        }
    }
    messageQueue_[1 - pushIndex_].clear();

    Tick();
}

