//
// Created by floweryclover on 2025-06-13.
//

#ifndef RATKINIASERVER_MESSAGEPOOL_H
#define RATKINIASERVER_MESSAGEPOOL_H

#include "RatkiniaProtocol.gen.h"
#include "Errors.h"
#include <array>
#include <vector>
#include <memory>

/**
 * Acquire()는 여러 생산자 스레드가 동시에 진행해도 안전,
 * Release()는 반드시 단일 소비자 스레드에서만 호출.
 */
class alignas(64) MpscMessageBodyPool final
{
public:
    explicit MpscMessageBodyPool() = default;

    ~MpscMessageBodyPool();

    MpscMessageBodyPool(const MpscMessageBodyPool&) = delete;

    MpscMessageBodyPool& operator=(const MpscMessageBodyPool&) = delete;

    MpscMessageBodyPool(MpscMessageBodyPool&&) = delete;

    MpscMessageBodyPool& operator=(MpscMessageBodyPool&&) = delete;

    char* Acquire(const size_t size)
    {
        const auto [head, blockSize]{ GetMemoryBlockListHeadFor(size) };
        if (!head)
        {
            ERR_PRINT_VARARGS(size, "bytes에 대한 GetMemoryBlockListHeadFor()가 실패했습니다.");
            std::abort();
        }

        auto expected{ head->load(std::memory_order_acquire) };
        while (expected != nullptr && !head->compare_exchange_weak(expected, expected->Next, std::memory_order_acq_rel, std::memory_order_acquire));

        if (expected)
        {
            return expected->Body;
        }

        const auto newBlock{ reinterpret_cast<MemoryBlock*>(::operator new(8 + blockSize)) };
        return newBlock->Body;
    }

    void Release(const size_t size, const char* const body)
    {
        const auto [head, blockSize]{ GetMemoryBlockListHeadFor(size) };
        if (!head)
        {
            ERR_PRINT_VARARGS(size, "bytes에 대한 GetMemoryBlockListHeadFor()가 실패했습니다.");
            std::abort();
        }

        const auto memoryBlockOfBuffer{ reinterpret_cast<MemoryBlock*>(reinterpret_cast<size_t>(body - offsetof(MemoryBlock, Body))) };

        auto expected{ head->load(std::memory_order_acquire) };
        do
        {
            memoryBlockOfBuffer->Next = expected;
        } while (!head->compare_exchange_weak(expected,
                                              memoryBlockOfBuffer,
                                              std::memory_order_acq_rel,
                                              std::memory_order_acquire));
    }

private:
    struct MemoryBlock
    {
        MemoryBlock* Next;
        char Body[];
    };

    struct alignas(64) AlignedAtomicMemoryBlockListHead
    {
        std::atomic<MemoryBlock*> Head;
    };

    static constexpr int PoolIndex_1_16 = 0;
    static constexpr int PoolIndex_17_32 = 1;
    static constexpr int PoolIndex_33_64 = 2;
    static constexpr int PoolIndex_65_128 = 3;
    static constexpr int PoolIndex_129_256 = 4;
    static constexpr int PoolIndex_257_512 = 5;
    static constexpr int PoolIndex_513_1024 = 6;
    static constexpr int PoolIndex_1025_MaxMessageBodySize = 7;

    std::array<AlignedAtomicMemoryBlockListHead, 8> memoryBlockListHeads_;

    std::pair<std::atomic<MemoryBlock*>*, size_t> GetMemoryBlockListHeadFor(const size_t size)
    {
        if (size > RatkiniaProtocol::MessageMaxSize - RatkiniaProtocol::MessageHeaderSize)
        {
            return { nullptr, 0 };
        }

        if (size > 1024)
        {
            return { &memoryBlockListHeads_[PoolIndex_1025_MaxMessageBodySize].Head, RatkiniaProtocol::MessageMaxSize - RatkiniaProtocol::MessageHeaderSize };
        }
        else if (size > 512)
        {
            return { &memoryBlockListHeads_[PoolIndex_513_1024].Head, 1024 };
        }
        else if (size > 256)
        {
            return { &memoryBlockListHeads_[PoolIndex_257_512].Head, 512 };
        }
        else if (size > 128)
        {
            return { &memoryBlockListHeads_[PoolIndex_129_256].Head, 256 };
        }
        else if (size > 64)
        {
            return { &memoryBlockListHeads_[PoolIndex_65_128].Head, 128 };
        }
        else if (size > 32)
        {
            return { &memoryBlockListHeads_[PoolIndex_33_64].Head, 64 };
        }
        else if (size > 16)
        {
            return { &memoryBlockListHeads_[PoolIndex_17_32].Head, 32 };
        }
        else
        {
            return { &memoryBlockListHeads_[PoolIndex_1_16].Head, 16 };
        }
    }
};

#endif //RATKINIASERVER_MESSAGEPOOL_H
