//
// Created by floweryclover on 2025-07-20.
// From Staticia Project
//

#ifndef SPARSEARRAY_H
#define SPARSEARRAY_H

#include "MakeIterableFromThis.h"
#include "ErrorMacros.h"
#include "MemoryPool/IndexedUnmanagedList.h"
#include <vector>

template<typename TData>
class SparseArray final : public MakeIterableFromThis<SparseArray<TData>>
{
    friend class MakeIterableFromThis<SparseArray>;
    using DataType = TData;

    using SparseBlock = uint32_t;
#pragma pack(push, 1)
    struct DenseBlock
    {
        SparseBlock SparseIndex;
        char Data[];
    };
#pragma pack(pop)

public:
    static constexpr uint32_t NullIndex = std::numeric_limits<uint32_t>::max();

    explicit SparseArray(const size_t densePageSize = 65536,
                         const size_t dataSize = sizeof(TData),
                         const size_t sparsePageSize = 16384)
        : DataSize{ dataSize },
          DenseBlocksPerPage{ densePageSize / (DenseBlockMetadataSize + dataSize) },
          DenseBlockSize{ DenseBlockMetadataSize + dataSize },
          count_{ 0 },
          shouldInvalidateIterator_{ false },
          sparseBlocks_{ sparsePageSize }
    {
    }

    ~SparseArray()
    {
        for (const auto densePage : densePages_)
        {
            if (!std::is_trivially_destructible_v<TData>)
            {
                for (int denseBlockIndex = 0; denseBlockIndex < DenseBlocksPerPage; ++denseBlockIndex)
                {
                    const auto denseBlock = reinterpret_cast<DenseBlock*>(
                        densePage + denseBlockIndex * DenseBlockSize);
                    reinterpret_cast<TData*>(denseBlock->Data)->~TData();
                }
            }
            operator delete(densePage);
        }
    }

    SparseArray(const SparseArray&) = delete;

    SparseArray& operator=(const SparseArray&) = delete;

    SparseArray& operator=(SparseArray&&) = delete;

    SparseArray(SparseArray&&) = delete;

    template<typename... Args>
    [[nodiscard]]
    std::pair<uint32_t, TData*> Emplace(Args&&... args)
    {
        const auto [sparseIndex, sparseBlock] = sparseBlocks_.Create();

        const auto denseIndex = count_;
        // ReSharper disable once CppDFANullDereference
        sparseBlock = denseIndex;
        count_ += 1;

        const size_t densePageIndex = denseIndex / DenseBlocksPerPage;
        const size_t denseBlockIndex = denseIndex % DenseBlocksPerPage;
        // ReSharper disable once CppDFALoopConditionNotUpdated
        while (densePageIndex >= densePages_.size())
        {
            densePages_.push_back(static_cast<char*>(operator new(DenseBlockSize * DenseBlocksPerPage)));
        }
        const auto denseBlock = reinterpret_cast<DenseBlock*>(
            densePages_[densePageIndex] + denseBlockIndex * DenseBlockSize);
        denseBlock->SparseIndex = sparseIndex;

        return { sparseIndex, new(denseBlock->Data) TData{ std::forward<Args>(args)... } };
    }

    void EraseBySparseIndex(const uint32_t sparseIndex)
    {
        const auto eraseSparseBlock = sparseBlocks_.Get(sparseIndex);
        CRASH_COND(!eraseSparseBlock);

        // ReSharper disable once CppDFANullDereference
        const uint32_t eraseDenseIndex = *eraseSparseBlock;
        const auto eraseDenseBlock = reinterpret_cast<DenseBlock*>(
            densePages_[eraseDenseIndex / DenseBlocksPerPage] + (eraseDenseIndex % DenseBlocksPerPage) *
            DenseBlockSize);

        // ReSharper disable once CppDFANullDereference
        *eraseSparseBlock = NullIndex;
        sparseBlocks_.Erase(sparseIndex);
        count_ -= 1;
        if constexpr (!std::is_trivially_destructible_v<TData>)
        {
            reinterpret_cast<TData*>(eraseDenseBlock->Data)->~TData();
        }

        if (eraseDenseIndex == count_) // Swap and pop 필요 없음
        {
            return;
        }

        const auto lastDenseBlock = reinterpret_cast<DenseBlock*>(
            densePages_[count_ / DenseBlocksPerPage] + (count_ % DenseBlocksPerPage) *
            DenseBlockSize);
        const auto lastSparseBlock = sparseBlocks_.Get(lastDenseBlock->SparseIndex);
        SCRASH_COND(!lastSparseBlock);
        // ReSharper disable once CppDFANullDereference
        *lastSparseBlock = eraseDenseIndex;

        if constexpr (std::is_trivially_copyable_v<TData>)
        {
            memcpy(eraseDenseBlock, lastDenseBlock, DenseBlockSize);
        }
        else
        {
            eraseDenseBlock->SparseIndex = lastDenseBlock->SparseIndex;
            new(eraseDenseBlock->Data) TData{ std::move(*reinterpret_cast<TData*>(lastDenseBlock->Data)) };
        }

        if constexpr (!std::is_trivially_destructible_v<TData>)
        {
            reinterpret_cast<TData*>(lastDenseBlock->Data)->~TData();
        }
    }

    [[nodiscard]]
    const TData* Get(const uint32_t sparseIndex) const
    {
        const auto sparseBlock = sparseBlocks_.Get(sparseIndex);
        if (!sparseBlock)
        {
            return nullptr;
        }

        const uint32_t denseIndex = *sparseBlock;
        const auto denseBlock = reinterpret_cast<DenseBlock*>(
            densePages_[denseIndex / DenseBlocksPerPage] + (denseIndex % DenseBlocksPerPage) *
            DenseBlockSize);
        return reinterpret_cast<const TData*>(denseBlock->Data);
    }

    [[nodiscard]]
    TData* Get(const uint32_t sparseIndex)
    {
        return const_cast<TData*>(static_cast<const SparseArray*>(this)->Get(sparseIndex));
    }

private:
    static constexpr size_t DenseBlockMetadataSize = offsetof(DenseBlock, Data);
    const size_t DataSize;
    const size_t DenseBlocksPerPage;
    const size_t DenseBlockSize;

    size_t count_;
    bool shouldInvalidateIterator_;
    std::vector<char*> densePages_;
    IndexedUnmanagedList<SparseBlock> sparseBlocks_;

    std::pair<uint32_t, const TData&> GetPairOfSparseIndexAndDataByDenseIndex(const uint32_t denseIndex) const
    {
        const auto denseBlock = reinterpret_cast<DenseBlock*>(
            densePages_[denseIndex / DenseBlocksPerPage] + (denseIndex % DenseBlocksPerPage) * DenseBlockSize);
        return { denseBlock->SparseIndex, *reinterpret_cast<const TData*>(denseBlock->Data) };
    }

    std::pair<uint32_t, TData&> GetPairOfSparseIndexAndDataByDenseIndex(const uint32_t denseIndex)
    {
        const auto [sparseIndex, constData] = static_cast<const SparseArray*>(this)->
            GetPairOfSparseIndexAndDataByDenseIndex(denseIndex);
        return { sparseIndex, const_cast<TData&>(constData) };
    }
};
#endif //SPARSEARRAY_H
