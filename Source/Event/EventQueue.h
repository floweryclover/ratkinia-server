//
// Created by floweryclover on 2025-08-27.
//

#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include <vector>

struct RawEventQueue
{
    virtual ~RawEventQueue() = default;

    virtual void Clear() = 0;
};

template<typename TEvent>
struct EventQueue final : RawEventQueue
{
    std::vector<TEvent> Events;

    void Clear() override
    {
        Events.clear();
    }
};

template<typename TEvent>
class EventRange final
{
public:
    explicit EventRange(std::vector<TEvent>& eventQueue)
        : eventQueue_{eventQueue}
    {}

    typename std::vector<TEvent>::iterator begin()
    {
        return eventQueue_.begin();
    }

    typename std::vector<TEvent>::iterator end()
    {
        return eventQueue_.end();
    }

private:
    std::vector<TEvent>& eventQueue_;
};

#endif //EVENTQUEUE_H
