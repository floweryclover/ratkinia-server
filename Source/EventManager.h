//
// Created by floweryclover on 2025-08-11.
//

#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "RuntimeOrder.h"
#include "Event_SessionErased.h"
#include <vector>

class EventManager final
{
public:
    explicit EventManager();

    ~EventManager() = default;

    EventManager(const EventManager&) = delete;

    EventManager(EventManager&&) = delete;

    EventManager& operator=(const EventManager&) = delete;

    EventManager& operator=(EventManager&&) = delete;

    void Clear()
    {
        for (auto& queue : eventQueues_)
        {
            queue.clear();
        }
    }

    template<typename TEvent, typename ...Args>
    void Push(Args&&... args)
    {
        CRASH_COND(TEvent::GetRuntimeOrder() == UnregisteredRuntimeOrder);
        eventQueues_[TEvent::GetRuntimeOrder()].emplace_back(std::forward<Args>(args)...);
    }

    template<typename TEvent>
    std::vector<Event_SessionErased>& Events()
    {
        CRASH_COND(TEvent::GetRuntimeOrder() == UnregisteredRuntimeOrder);
        return eventQueues_[TEvent::GetRuntimeOrder()];
    }

    template<typename TEvent>
    const std::vector<Event_SessionErased>& Events() const
    {
        CRASH_COND(TEvent::GetRuntimeOrder() == UnregisteredRuntimeOrder);
        return eventQueues_[TEvent::GetRuntimeOrder()];
    }

private:
    std::vector<std::vector<Event_SessionErased>> eventQueues_;
};

#endif //EVENTMANAGER_H
