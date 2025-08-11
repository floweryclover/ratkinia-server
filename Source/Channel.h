//
// Created by floweryclover on 2025-05-08.
//

#ifndef RATKINIASERVER_CHANNEL_H
#define RATKINIASERVER_CHANNEL_H

#include <atomic>
#include <memory>
#include <optional>

// template<typename TSender, typename TReceiver>
// class Channel
// {
// public:
//     virtual ~Channel() = default;
//
//     std::pair<TSender, TReceiver> CreateChannel();
//
// private:
//     bool isChannelCreated_ = false;
// };

//
// template<typename TChannel>
// class CreateMpscChannelFromThis
// {
//     struct alignas(64) MpscChannel final
//     {
//         TChannel Channel{};
//         std::atomic_bool Closed{};
//         std::atomic_uint32_t Version{};
//         std::atomic_size_t SenderCount{};
//     };
//
// public:
//     class MpscSender final
//     {
//     public:
//         explicit MpscSender(std::shared_ptr<MpscChannel> channel)
//             : channel_{ std::move(channel) }
//         {
//             channel_->SenderCount.fetch_add(1, std::memory_order_release);
//         }
//
//         ~MpscSender()
//         {
//             if (channel_ != nullptr && channel_->SenderCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
//             {
//                 channel_->Closed.store(true, std::memory_order_release);
//                 channel_->Version.fetch_add(1, std::memory_order_release);
//                 channel_->Version.notify_one();
//             }
//         }
//
//         MpscSender(const MpscSender&) = default;
//
//         MpscSender& operator=(const MpscSender&) = default;
//
//         MpscSender(MpscSender&&) noexcept = default;
//
//         MpscSender& operator=(MpscSender&&) noexcept = default;
//
//         template<typename ...Args>
//         bool TryPush(Args&& ...args)
//         {
//             if (channel_->Closed.load(std::memory_order_acquire))
//             {
//                 return false;
//             }
//
//             const auto pushResult = channel_->Channel.TryPush(std::forward<Args>(args)...);
//             channel_->Version.fetch_add(1, std::memory_order_release);
//             channel_->Version.notify_one();
//             return pushResult;
//         }
//
//     private:
//         std::shared_ptr<MpscChannel> channel_;
//     };
//
//     class MpscReceiver final
//     {
//     public:
//         explicit MpscReceiver(std::shared_ptr<MpscChannel> channel)
//             : channel_{ std::move(channel) }
//         {
//         }
//
//         ~MpscReceiver()
//         {
//             if (channel_)
//             {
//                 channel_->Closed.store(true, std::memory_order_release);
//                 channel_->Version.fetch_add(1, std::memory_order_release);
//                 channel_->Version.notify_one();
//             }
//         }
//
//         MpscReceiver(const MpscReceiver&) = delete;
//
//         MpscReceiver& operator=(const MpscReceiver) = delete;
//
//         MpscReceiver(MpscReceiver&& rhs)
//             : channel_{ std::move(rhs.channel_) }
//         {
//             rhs.channel_ = nullptr;
//         }
//
//         MpscReceiver& operator=(MpscReceiver&&) noexcept = default;
//
//         void Wait()
//         {
//             while (true)
//             {
//                 if (channel_->Closed.load(std::memory_order_acquire)
//                     || !channel_->Channel.Empty())
//                 {
//                     return;
//                 }
//
//                 channel_->Version.wait(loadedVersion_, std::memory_order_acquire);
//                 loadedVersion_ = channel_->Version.load(std::memory_order_acquire);
//             }
//         }
//
//         [[nodiscard]]
//         bool IsClosed() const
//         {
//             return channel_->Closed.load(std::memory_order_acquire);
//         }
//
//         typename TChannel::ChannelPeekOutputType TryPeek()
//         {
//             return channel_->Channel.TryPeek();
//         }
//
//         void Pop(TChannel::ChannelPopInputType popInput)
//         {
//             channel_->Channel.Pop(popInput);
//         }
//
//     private:
//         std::shared_ptr<MpscChannel> channel_;
//         uint32_t loadedVersion_;
//     };
//
//     [[nodiscard]]
//     static std::pair<MpscSender, MpscReceiver> CreateMpscChannel()
//     {
//         auto channel = std::make_shared<MpscChannel>();
//         auto sender = MpscSender{ channel };
//         auto receiver = MpscReceiver{ std::move(channel) };
//
//         return { std::move(sender), std::move(receiver) };
//     }
// };
//
// template<typename TChannel>
// class CreateSpscChannelFromThis
// {
//     struct alignas(64) SpscChannel final
//     {
//         TChannel Channel{};
//         std::atomic_bool Closed{};
//         std::atomic_uint32_t Version{};
//     };
//
// public:
//     class SpscSender final
//     {
//     public:
//         explicit SpscSender(std::shared_ptr<SpscChannel> channel)
//             : channel_{ std::move(channel) }
//         {}
//
//         ~SpscSender()
//         {
//             if (channel_ != nullptr && !channel_->Closed.exchange(true, std::memory_order_acq_rel))
//             {
//                 channel_->Version.fetch_add(1, std::memory_order_release);
//                 channel_->Version.notify_one();
//             }
//         }
//
//         SpscSender(const SpscSender&) = delete;
//
//         SpscSender& operator=(const SpscSender&) = delete;
//
//         SpscSender(SpscSender&&) noexcept = default;
//
//         SpscSender& operator=(SpscSender&&) noexcept = default;
//
//         template<typename ...Args>
//         bool TryPush(Args&& ...args)
//         {
//             if (channel_->Closed.load(std::memory_order_acquire))
//             {
//                 return false;
//             }
//
//             const auto pushResult = channel_->Channel.TryPush(std::forward<Args>(args)...);
//             channel_->Version.fetch_add(1, std::memory_order_release);
//             channel_->Version.notify_one();
//             return pushResult;
//         }
//
//     private:
//         std::shared_ptr<SpscChannel> channel_;
//     };
//
//     class SpscReceiver final
//     {
//     public:
//         explicit SpscReceiver(std::shared_ptr<SpscChannel> channel)
//             : channel_{ std::move(channel) }
//         {
//         }
//
//         ~SpscReceiver()
//         {
//             if (channel_)
//             {
//                 channel_->Closed.store(true, std::memory_order_release);
//                 channel_->Version.fetch_add(1, std::memory_order_release);
//                 channel_->Version.notify_one();
//             }
//         }
//
//         SpscReceiver(const SpscReceiver&) = delete;
//
//         SpscReceiver& operator=(const SpscReceiver) = delete;
//
//         SpscReceiver(SpscReceiver&& rhs)
//             : channel_{ std::move(rhs.channel_) }
//         {
//             rhs.channel_ = nullptr;
//         }
//
//         SpscReceiver& operator=(SpscReceiver&&) noexcept = default;
//
//         void Wait()
//         {
//             while (true)
//             {
//                 if (channel_->Closed.load(std::memory_order_acquire)
//                     || !channel_->Channel.Empty())
//                 {
//                     return;
//                 }
//
//                 channel_->Version.wait(loadedVersion_, std::memory_order_acquire);
//                 loadedVersion_ = channel_->Version.load(std::memory_order_acquire);
//             }
//         }
//
//         [[nodiscard]]
//         bool IsClosed() const
//         {
//             return channel_->Closed.load(std::memory_order_acquire);
//         }
//
//         TChannel::ChannelPeekOutputType TryPeek()
//         {
//             return channel_->Channel.TryPeek();
//         }
//
//         void Pop(TChannel::ChannelPopInputType popInput)
//         {
//             channel_->Channel.Pop(popInput);
//         }
//
//     private:
//         std::shared_ptr<SpscChannel> channel_;
//         uint32_t loadedVersion_;
//     };
//
//     [[nodiscard]]
//     static std::pair<SpscSender, SpscReceiver> CreateSpscChannel()
//     {
//         auto channel = std::make_shared<SpscChannel>();
//         auto sender = SpscSender{ channel };
//         auto receiver = SpscReceiver{ std::move(channel) };
//
//         return { std::move(sender), std::move(receiver) };
//     }
// };

#endif //RATKINIASERVER_CHANNEL_H
