//
// Created by floweryclover on 2025-08-11.
//

#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "Manager.h"
#include "EventQueue.h"
#include "ErrorMacros.h"
#include "RuntimeOrder.h"
#include <vector>
#include <memory>

class EventManager final : public Manager
{
public:
    template<typename TEvent>
    void Register()
    {
        TEvent::SetRuntimeOrder(eventQueues_.size());
        eventQueues_.emplace_back(std::make_unique<EventQueue<TEvent>>());
    }

    void Clear()
    {
        for (auto& queue : eventQueues_)
        {
            queue->Clear();
        }
    }

    template<typename TEvent, typename... Args>
    TEvent& Push(Args&&... args)
    {
        return GetEventQueue<TEvent>().emplace_back(std::forward<Args>(args)...);
    }

    template<typename TEvent>
    EventRange<TEvent> Events()
    {
        return EventRange<TEvent>{ GetEventQueue<TEvent>() };
    }

private:
    std::vector<std::unique_ptr<RawEventQueue>> eventQueues_;

    template<typename TEvent>
    std::vector<TEvent>& GetEventQueue()
    {
        CRASH_COND(TEvent::GetRuntimeOrder() == UnregisteredRuntimeOrder);
        return static_cast<EventQueue<TEvent>&>(*eventQueues_[TEvent::GetRuntimeOrder()]).Events;
    }
};

#endif //EVENTMANAGER_H
