//
// Created by floweryclover on 2025-06-26.
//

#ifndef RATKINIASERVER_SPSCRINGBUFFER_H
#define RATKINIASERVER_SPSCRINGBUFFER_H

#include <atomic>
#include <optional>
#include <memory>

/**
 * 사용 측에서 Spsc를 보장할 것, Spsc가 보장되면 스레드 안전하게 동작함.
 */
class alignas(64) SpscRingBuffer final
{
private:
    struct BufferStatus
    {
        size_t LoadedHead{};
        size_t LoadedTail{};
        size_t Size{};
        size_t AvailableSize{};
    };

public:
    const size_t Capacity;
    const size_t MaxBlockSize;

    /**
     *
     * @param capacity
     * @param maxBlockSize 불연속 구간에 대한 작업시 임시 버퍼에 복사해야 하므로, 이 임시 버퍼를 위한 크기.
     */
    explicit SpscRingBuffer(size_t capacity, size_t maxBlockSize);

    ~SpscRingBuffer() = default;

    SpscRingBuffer(const SpscRingBuffer&) = delete;

    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;

    SpscRingBuffer(SpscRingBuffer&&) = delete;

    SpscRingBuffer& operator=(SpscRingBuffer&&) = delete;

    template<typename T>
    bool TryEnqueue(const T& data, const size_t size)
    {
        const auto bufferStatus = GetBufferStatus();
        if (bufferStatus.AvailableSize < size) [[unlikely]]
        {
            return false;
        }

        const auto primarySize{ std::min<size_t>(size, Capacity - bufferStatus.LoadedTail) };
        const auto secondarySize{ size - primarySize };

        if constexpr (std::is_convertible_v<T, const char*>)
        {
            memcpy(ringBuffer_.get() + bufferStatus.LoadedTail, data, primarySize);
            memcpy(ringBuffer_.get(), reinterpret_cast<const char*>(data) + primarySize, secondarySize);
        }
        else
        {
            if (secondarySize == 0) [[likely]]
            {
                data.SerializeToArray(ringBuffer_.get() + bufferStatus.LoadedTail, size);
            }
            else [[unlikely]]
            {
                data.SerializeToArray(tempEnqueueBuffer_.get(), size);
                memcpy(ringBuffer_.get() + bufferStatus.LoadedTail, tempEnqueueBuffer_.get(), primarySize);
                memcpy(ringBuffer_.get(), tempEnqueueBuffer_.get() + primarySize, secondarySize);
            }
        }

        tail_.store((bufferStatus.LoadedTail + size) % Capacity, std::memory_order_release);

        return true;
    }

    void Dequeue(const size_t size)
    {
        head_.store((head_.load(std::memory_order_acquire) + size) % Capacity, std::memory_order_release);
    }

    std::optional<const char*> TryPeek(const size_t size)
    {
        const auto bufferStatus = GetBufferStatus();
        if (bufferStatus.Size < size)
        {
            return std::nullopt;
        }

        if (Capacity - bufferStatus.LoadedHead >= size) [[likely]] // 연속 구간이면 그대로 반환
        {
            return ringBuffer_.get() + bufferStatus.LoadedHead;
        }

        // 불연속 구간이면 임시 버퍼에 복사 후 반환
        const auto primarySize{ Capacity - bufferStatus.LoadedHead };
        const auto secondarySize{ size - primarySize };
        memcpy(tempDequeueBuffer_.get(), ringBuffer_.get() + bufferStatus.LoadedHead, primarySize);
        memcpy(tempDequeueBuffer_.get() + primarySize, ringBuffer_.get(), secondarySize);

        return tempDequeueBuffer_.get();
    }

    size_t GetAvailableSize() const
    {
        return GetBufferStatus().AvailableSize;
    }


    size_t GetSize() const
    {
        return GetBufferStatus().Size;
    }

private:
    std::unique_ptr<char[]> ringBuffer_;
    std::unique_ptr<char[]> tempDequeueBuffer_;
    std::unique_ptr<char[]> tempEnqueueBuffer_;

    alignas(64) std::atomic<size_t> head_{}; // inclusive
    alignas(64) std::atomic<size_t> tail_{}; // exclusive, head_와 동일하면 empty.

    BufferStatus GetBufferStatus() const
    {
        const auto loadedHead{ head_.load(std::memory_order_acquire) };
        const auto loadedTail{ tail_.load(std::memory_order_acquire) };

        const auto size{ loadedTail >= loadedHead ? loadedTail - loadedHead : loadedTail + Capacity - loadedHead };

        return { loadedHead, loadedTail, size, Capacity - size - 1 };
    }
};

#endif //RATKINIASERVER_SPSCRINGBUFFER_H
