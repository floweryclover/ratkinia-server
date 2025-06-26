//
// Created by floweryclover on 2025-06-26.
//

#ifndef RATKINIASERVER_SPSCRINGBUFFER_H
#define RATKINIASERVER_SPSCRINGBUFFER_H

#include "ScopedBufferHandle.h"
#include <atomic>
#include <optional>
#include <memory>

/**
 * 사용 측에서 Spsc를 보장할 것, Spsc가 보장되면 스레드 안전하게 동작함.
 */
class alignas(64) SpscRingBuffer final
{
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

    __forceinline bool TryEnqueue(const char* const data, const size_t size)
    {
        const auto [loadedSize, loadedHead, loadedTail]{ GetSize() };
        const auto availableSize{ Capacity - loadedSize - 1 };

        if (availableSize < size) [[unlikely]]
        {
            return false;
        }

        const auto primarySize{ std::min<size_t>(size, Capacity - loadedTail) };
        const auto secondarySize{ size - primarySize };
        memcpy(ringBuffer_.get() + loadedTail, data, primarySize);
        memcpy(ringBuffer_.get(), data + primarySize, secondarySize);

        tail_.store((loadedTail + size) % Capacity, std::memory_order_release);

        return true;
    }

    __forceinline std::optional<ScopedBufferEnqueuer<SpscRingBuffer>> TryGetEnqueuer(const size_t size)
    {
        const auto [loadedSize, loadedHead, loadedTail]{ GetSize() };
        const auto availableSize{Capacity - loadedSize - 1};
        if (availableSize < size) [[unlikely]]
        {
            return std::nullopt;
        }

        // 연속 구간에 대해 쓰기 가능하면 즉시 해당 위치 반환
        if (Capacity - loadedTail >= size) [[likely]]
        {
            return std::make_optional<ScopedBufferEnqueuer<SpscRingBuffer>>(this, ringBuffer_.get() + loadedTail, size);
        }

        // 불연속 구간인 경우 임시 버퍼에 대해 반환
        return std::make_optional<ScopedBufferEnqueuer<SpscRingBuffer>>(this, tempEnqueueBuffer_.get(), size);
    }

    void Dequeue(const size_t size)
    {
        head_.store((head_.load(std::memory_order_acquire) + size) % Capacity, std::memory_order_release);
    }

    __forceinline std::optional<const char*> TryPeek(const size_t size)
    {
        const auto [bufferSize, loadedHead, loadedTail]{ GetSize() };
        if (bufferSize < size)
        {
            return std::nullopt;
        }

        if (Capacity - loadedHead >= size) [[likely]] // 연속 구간이면 그대로 반환
        {
            return ringBuffer_.get() + loadedHead;
        }

        // 불연속 구간이면 임시 버퍼에 복사 후 반환
        const auto primarySize{ Capacity - loadedHead };
        const auto secondarySize{ size - primarySize };
        memcpy(tempDequeueBuffer_.get(), ringBuffer_.get() + loadedHead, primarySize);
        memcpy(tempDequeueBuffer_.get() + primarySize, ringBuffer_.get(), secondarySize);

        return tempDequeueBuffer_.get();
    }

    /**
     *
     * @return [size, loadedHead, loadedTail] 잦은 atomic load 방지를 위해 head, tail도 같이 반환.
     */
    __forceinline std::tuple<size_t, size_t, size_t> GetSize() const
    {
        const auto loadedHead{ head_.load(std::memory_order_acquire) };
        const auto loadedTail{ tail_.load(std::memory_order_acquire) };

        const auto size{ loadedTail >= loadedHead ? loadedTail - loadedHead : loadedTail + Capacity - loadedHead };

        return { size, loadedHead, loadedTail };
    }

    __forceinline void ReleaseEnqueueBuffer(const char* const buffer, const size_t bufferSize)
    {
        const auto [loadedSize, loadedHead, loadedTail]{GetSize()};
        if (buffer == tempEnqueueBuffer_.get()) [[unlikely]]
        {
            const auto primarySize{ std::min<size_t>(bufferSize, Capacity - loadedTail) };
            const auto secondarySize{ bufferSize - primarySize };
            memcpy(ringBuffer_.get() + loadedTail, buffer, primarySize);
            memcpy(ringBuffer_.get(), buffer + primarySize, secondarySize);

        }

        tail_.store((loadedTail + bufferSize) % Capacity, std::memory_order_release);
    }

private:
    alignas(64) std::unique_ptr<char[]> ringBuffer_;
    alignas(64) std::unique_ptr<char[]> tempDequeueBuffer_;
    alignas(64) std::unique_ptr<char[]> tempEnqueueBuffer_;

    alignas(64) std::atomic<size_t> head_{}; // inclusive
    alignas(64) std::atomic<size_t> tail_{}; // exclusive, head_와 동일하면 empty.
};

#endif //RATKINIASERVER_SPSCRINGBUFFER_H
