//
// Created by floweryclover on 2025-05-08.
//

#ifndef RATKINIASERVER_MAINSERVERPIPE_H
#define RATKINIASERVER_MAINSERVERPIPE_H

#include <mutex>
#include <queue>
#include <cstdint>

class MainServerPipe final
{
public:
    enum class Command
    {
        Shutdown
    };

    using Message = Command;

    bool Push(Message message)
    {
        std::lock_guard lock{mutex_};
        commands_.emplace(message);
        return true;
    }

    bool TryPeek(Message& outMessage)
    {
        std::lock_guard lock{mutex_};
        if (commands_.empty())
        {
            return false;
        }

        outMessage = commands_.front();
        return true;
    }

    void Pop()
    {
        std::lock_guard lock{mutex_};
        if (!commands_.empty())
        {
            commands_.pop();
        }
    }

    bool Empty()
    {
        std::lock_guard lock{mutex_};
        return commands_.empty();
    }

private:
    std::mutex mutex_;
    std::queue<Command> commands_;
};


#endif //RATKINIASERVER_MAINSERVERPIPE_H
