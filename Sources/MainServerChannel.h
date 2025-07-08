//
// Created by floweryclover on 2025-05-08.
//

#ifndef RATKINIASERVER_MAINSERVERCHANNEL_H
#define RATKINIASERVER_MAINSERVERCHANNEL_H

#include "Channel.h"
#include <mutex>
#include <queue>
#include <optional>
#include <cstdint>

class MainServerCommand
{
public:
    virtual ~MainServerCommand() = 0;
};

class ShutdownCommand final : public MainServerCommand
{
public:
    ~ShutdownCommand() override = default;
};

class MainServerChannel final : public CreateMpscChannelFromThis<MainServerChannel>
{
public:
    using ChannelPeekOutputType = const MainServerCommand*;
    using ChannelPopInputType = const MainServerCommand*;

    bool TryPush(std::unique_ptr<MainServerCommand> command)
    {
        std::lock_guard lock{ mutex_ };
        commands_.emplace(std::move(command));
        return true;
    }

    ChannelPeekOutputType TryPeek()
    {
        std::lock_guard lock{ mutex_ };
        if (commands_.empty())
        {
            return nullptr;
        }

        return commands_.front().get();
    }

    void Pop(ChannelPopInputType popInput)
    {
        commands_.pop();
    }

    bool Empty()
    {
        std::lock_guard lock{ mutex_ };
        return commands_.empty();
    }

private:
    std::mutex mutex_;
    std::queue<std::unique_ptr<MainServerCommand>> commands_;
};


#endif //RATKINIASERVER_MAINSERVERCHANNEL_H
