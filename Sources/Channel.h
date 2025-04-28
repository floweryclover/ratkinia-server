//
// Created by floweryclover on 2025-05-08.
//

#ifndef RATKINIASERVER_CHANNEL_H
#define RATKINIASERVER_CHANNEL_H

#include <atomic>
#include <memory>

template<typename TPipe>
struct MpscChannel final
{
    TPipe Pipe;
    std::atomic_bool Closed;
    std::atomic_uint32_t Version;
    std::atomic_size_t SenderCount;
};

template<typename TPipe>
class MpscSender final
{
public:
    using Message = TPipe::Message;

    explicit MpscSender(std::shared_ptr<MpscChannel<TPipe>> channel)
        : channel_{ std::move(channel) }
    {
        channel_->SenderCount.fetch_add(1, std::memory_order_release);
    }

    ~MpscSender()
    {
        if (channel_ != nullptr && channel_->SenderCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            channel_->Closed.store(true, std::memory_order_release);
            channel_->Version.fetch_add(1, std::memory_order_release);
            channel_->Version.notify_one();
        }
    }

    MpscSender(const MpscSender&) = default;

    MpscSender& operator=(const MpscSender&) = default;

    MpscSender(MpscSender&&) noexcept = default;

    MpscSender& operator=(MpscSender&&) noexcept = default;

    bool Send(Message&& message)
    {
        if (channel_->Closed.load(std::memory_order_acquire))
        {
            return false;
        }

        return channel_->Pipe.Push(std::forward<Message>(message));
    }

private:
    std::shared_ptr<MpscChannel<TPipe>> channel_;
};

template<typename TPipe>
class MpscReceiver final
{
public:
    using Message = TPipe::Message;

    explicit MpscReceiver(std::shared_ptr<MpscChannel<TPipe>> channel)
        : channel_{ std::move(channel) }
    {
    }

    ~MpscReceiver()
    {
        if (channel_)
        {
            channel_->Closed.store(true, std::memory_order_release);
            channel_->Version.fetch_add(1, std::memory_order_release);
            channel_->Version.notify_one();
        }
    }

    MpscReceiver(const MpscReceiver&) = delete;

    MpscReceiver& operator=(const MpscReceiver) = delete;

    MpscReceiver(MpscReceiver&& rhs)
    : channel_{std::move(rhs.channel_)}
    {
        rhs.channel_ = nullptr;
    }

    MpscReceiver& operator=(MpscReceiver&&) noexcept = default;

    void Wait()
    {
        while (true)
        {
            if (channel_->Closed.load(std::memory_order_acquire)
                || channel_->Pipe.Empty())
            {
                return;
            }

            channel_->Version.wait(loadedVersion_, std::memory_order_acquire);
            loadedVersion_ = channel_->Version.load(std::memory_order_acquire);
        }
    }

    [[nodiscard]]
    bool Closed() const
    {
        return channel_->Closed.load(std::memory_order_acquire);
    }

    bool TryPeek(Message& outMessage)
    {
        return channel_->Pipe.TryPeek(outMessage);
    }

    void Pop()
    {
        channel_->Pipe.Pop();
    }

private:
    std::shared_ptr<MpscChannel<TPipe>> channel_;
    uint32_t loadedVersion_;
};

template<typename TPipe>
std::pair<MpscSender<TPipe>, MpscReceiver<TPipe>> CreateMpscChannel()
{
    auto channel = std::make_shared<MpscChannel<TPipe>>();
    auto sender = MpscSender<TPipe>{ channel };
    auto receiver = MpscReceiver<TPipe>{ std::move(channel) };

    return { std::move(sender), std::move(receiver) };
}

#endif //RATKINIASERVER_CHANNEL_H
