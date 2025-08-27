//
// Created by floweryclover on 2025-07-20.
// From Staticia Project.
//

#ifndef SPARSELIST_H
#define SPARSELIST_H

#include "IndexedUnmanagedList.h"
#include "MakeIterableFromThis.h"
#include "ErrorMacros.h"
#include <vector>

template<typename TData>
class SparseList final : public MakeIterableFromThis<SparseList<TData>>
{
    friend class MakeIterableFromThis<SparseList>;

#pragma pack(push, 1)
    struct SparseBlock
    {
        uint32_t DenseIndex;
        char Data[];
    };
#pragma pack(pop)
    struct SparseBlockInitializer
    {
        void operator()(const uint32_t, SparseBlock& sparseBlock)
        {
            sparseBlock.DenseIndex = NullIndex;
        }
    };

    using DenseBlock = uint32_t;

public:
    using DataType = TData;

    static constexpr uint32_t NullIndex = std::numeric_limits<uint32_t>::max();

    explicit SparseList(const size_t sparsePageSize = 65536,
                        const size_t dataSize = sizeof(TData),
                        const size_t densePageSize = 16384)
        : DataSize{ dataSize },
          DenseBlocksPerPage{ densePageSize / DenseBlockSize },
          count_{ 0 },
          shouldInvalidateIterator_{ false },
          sparseBlocks_{ sparsePageSize, sizeof(SparseBlock) + dataSize }
    {
    }

    ~SparseList()
    {
        for (int denseIndex = 0; denseIndex < count_; ++denseIndex)
        {
            const size_t densePageIndex = denseIndex / DenseBlocksPerPage;
            const size_t denseBlockIndex = denseIndex % DenseBlocksPerPage;
            const auto denseBlock = reinterpret_cast<DenseBlock*>(
                densePages_[densePageIndex] + (denseBlockIndex * DenseBlockSize));

            const uint32_t sparseIndex = *denseBlock;
            const auto sparseBlock = sparseBlocks_.Get(sparseIndex);
            reinterpret_cast<TData*>(sparseBlock->Data)->~TData();
        }

        for (const auto densePage : densePages_)
        {
            operator delete(densePage);
        }
    }

    SparseList(const SparseList&) = delete;

    SparseList& operator=(const SparseList&) = delete;

    SparseList(SparseList&&) = delete;

    SparseList& operator=(SparseList&&) = delete;

    template<typename... Args>
    [[nodiscard]]
    std::pair<uint32_t, TData*> Emplace(Args&&... args)
    {
        const auto [sparseIndex, sparseBlock] = sparseBlocks_.Create();
        const auto data = EmplaceImpl(sparseIndex, &sparseBlock, std::forward<Args>(args)...);
        return { sparseIndex, data };
    }

    template<typename... Args>
    TData* EmplaceAt(const uint32_t sparseIndex, Args&&... args)
    {
        const auto sparseBlock = sparseBlocks_.CreateAt(sparseIndex);
        CRASH_COND_MSG(sparseBlock == nullptr, sparseIndex);

        return EmplaceImpl(sparseIndex, sparseBlock, std::forward<Args>(args)...);
    }

    void EraseBySparseIndex(const uint32_t sparseIndex)
    {
        const auto sparseBlock = sparseBlocks_.Get(sparseIndex);
        CRASH_COND(!sparseBlock);
        if constexpr (!std::is_trivially_destructible_v<TData>)
        {
            reinterpret_cast<TData*>(sparseBlock->Data)->~TData();
        }

        const uint32_t eraseDenseIndex = sparseBlock->DenseIndex;
        sparseBlock->DenseIndex = NullIndex;
        sparseBlocks_.Erase(sparseIndex);
        count_ -= 1;

        if ([[maybe_unused]] const bool wasLastDenseBlock = eraseDenseIndex == count_)
        {
            return;
        }

        const size_t eraseDensePageIndex = eraseDenseIndex / DenseBlocksPerPage;
        const size_t eraseDenseBlockIndex = eraseDenseIndex % DenseBlocksPerPage;
        const auto eraseDenseBlock = eraseDensePageIndex < densePages_.size()
                                         ? reinterpret_cast<DenseBlock*>(
                                             densePages_[eraseDensePageIndex] + eraseDenseBlockIndex *
                                             DenseBlockSize)
                                         : nullptr;
        CRASH_COND(!eraseDenseBlock);

        const size_t swapDenseIndex = count_;
        const size_t swapDensePageIndex = swapDenseIndex / DenseBlocksPerPage;
        const size_t swapDenseBlockIndex = swapDenseIndex % DenseBlocksPerPage;
        const auto swapDenseBlock = reinterpret_cast<DenseBlock*>(
            densePages_[swapDensePageIndex] + swapDenseBlockIndex * DenseBlockSize);
        const auto swapSparseIndex = *swapDenseBlock;
        const auto swapSparseBlock = sparseBlocks_.Get(swapSparseIndex);

        // ReSharper disable once CppDFANullDereference
        *eraseDenseBlock = *swapDenseBlock;
        swapSparseBlock->DenseIndex = eraseDenseIndex;
    }

    [[nodiscard]]
    const TData* Get(const uint32_t sparseIndex) const
    {
        const auto sparseBlock = sparseBlocks_.Get(sparseIndex);
        return sparseBlock ? reinterpret_cast<const TData*>(sparseBlock->Data) : nullptr;
    }

    [[nodiscard]]
    TData* Get(const uint32_t sparseIndex)
    {
        return const_cast<TData*>(static_cast<const SparseList*>(this)->Get(sparseIndex));
    }

private:
    static constexpr size_t DenseBlockSize = sizeof(DenseBlock);

    const size_t DataSize;
    const size_t DenseBlocksPerPage;

    size_t count_;
    bool shouldInvalidateIterator_;
    IndexedUnmanagedList<SparseBlock> sparseBlocks_;
    std::vector<char*> densePages_;

    std::pair<uint32_t, const TData&> GetPairOfSparseIndexAndDataByDenseIndex(const uint32_t denseIndex) const
    {
        const auto denseBlock = reinterpret_cast<DenseBlock*>(
            densePages_[denseIndex / DenseBlocksPerPage] + (denseIndex % DenseBlocksPerPage) *
            DenseBlockSize);

        return {
            *denseBlock,
            *reinterpret_cast<const TData*>(sparseBlocks_.Get(*denseBlock)->Data)
        };
    }

    std::pair<uint32_t, TData&> GetPairOfSparseIndexAndDataByDenseIndex(const uint32_t denseIndex)
    {
        const auto [sparseIndex, nonConstData] = static_cast<const SparseList*>(this)->
            GetPairOfSparseIndexAndDataByDenseIndex(denseIndex);
        return { sparseIndex, const_cast<TData&>(nonConstData) };
    }

    template<typename... Args>
    TData* EmplaceImpl(const uint32_t sparseIndex, SparseBlock* const sparseBlock, Args&&... args)
    {
        const auto denseIndex = count_;
        // ReSharper disable once CppDFANullDereference
        sparseBlock->DenseIndex = denseIndex;
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
        *denseBlock = sparseIndex;
        return new(sparseBlock->Data) TData{ std::forward<Args>(args)... };
    }
};

#endif // SPARSELIST_H
