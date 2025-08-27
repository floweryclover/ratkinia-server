//
// Created by floweryclover on 2025-07-25.
// From Staticia Project.
//

#ifndef MAKEITERABLEFROMTHIS_H
#define MAKEITERABLEFROMTHIS_H

#include <type_traits>
#include <iterator>
#include <limits>

template<typename TPagedDenseSparseContainer>
class MakeIterableFromThis
{
public:
    template<bool Const>
    class IteratorBase final
    {
        using DataType = std::conditional_t<Const, const typename TPagedDenseSparseContainer::DataType&, typename
            TPagedDenseSparseContainer::DataType&>;
        using ContainerType = std::conditional_t<Const, const TPagedDenseSparseContainer&, TPagedDenseSparseContainer&>;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<uint32_t, DataType>;

        explicit IteratorBase(ContainerType container, const uint32_t denseIndex)
            : container_{ container },
              denseIndex_{ denseIndex }
        {
        }

        bool operator==(const IteratorBase&) const
        {
            // == end()이다. oob이다.
            return denseIndex_ >= container_.count_;
        }

        bool operator!=(const IteratorBase& rhs) const
        {
            return !(*this == rhs);
        }

        IteratorBase& operator=(const IteratorBase&) = default;

        value_type operator*() const
        {
            return container_.GetPairOfSparseIndexAndDataByDenseIndex(denseIndex_);
        }

        IteratorBase& operator++()
        {
            denseIndex_ = denseIndex_ + 1;
            if constexpr (!Const)
            {
                denseIndex_ -= static_cast<int>(container_.shouldInvalidateIterator_);
                container_.shouldInvalidateIterator_ = false;
            }

            return *this;
        }

    private:
        ContainerType container_;
        uint32_t denseIndex_;
    };

    using Iterator = IteratorBase<false>;

    using ConstIterator = IteratorBase<true>;

    Iterator begin()
    {
        shouldInvalidateIterator_ = false;
        return Iterator{ *static_cast<TPagedDenseSparseContainer*>(this), 0 };
    }

    Iterator end()
    {
        return Iterator{ *static_cast<TPagedDenseSparseContainer*>(this), std::numeric_limits<uint32_t>::max() };
    }

    ConstIterator begin() const
    {
        return cbegin();
    }

    ConstIterator end() const
    {
        return cend();
    }

    ConstIterator cbegin() const
    {
        return ConstIterator{ *static_cast<const TPagedDenseSparseContainer*>(this), 0 };
    }

    ConstIterator cend() const
    {
        return ConstIterator{
            *static_cast<const TPagedDenseSparseContainer*>(this), std::numeric_limits<uint32_t>::max()
        };
    }

protected:
    bool shouldInvalidateIterator_ = false;
};

#endif //MAKEITERABLEFROMTHIS_H
