//
// Created by floweryclover on 2025-07-21.
// From Staticia Project.
//

#ifndef INDEXEDUNMANAGEDLIST_H
#define INDEXEDUNMANAGEDLIST_H

#include <vector>

/**
 * Index 기반으로 조작하며, 생성자/소멸자 호출 없고, 내부 데이터 조작 없는 리스트.
 * @remarks - 생성자/소멸자 조작은 하지 않으므로 사용자 측에 책임 있음.
 * @remarks - 페이지 할당 시점 이후에는 어떠한 내부 데이터 조작도 없음 - 사용자가 상태 저장을 위해 사용 가능.
 */
template<typename TData, typename TBlockInitializerWhenPageAllocated=decltype([](const uint32_t, TData&)
{
})>
class IndexedUnmanagedList final
{
#pragma pack(push, 1)
    struct Node
    {
        uint32_t Index;
        bool Allocated;
        Node* Next;
        Node* Prev;
        char Data[];
    };
#pragma pack(pop)

public:
    static constexpr size_t NodeMetadataSize = offsetof(Node, Data);

    template<bool Const>
    class IteratorBase final
    {
    public:
        using DataType = std::conditional_t<Const, const TData&, TData&>;
        using ContainerType = std::conditional_t<Const, const IndexedUnmanagedList&, IndexedUnmanagedList&>;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<uint32_t, DataType>;

        explicit IteratorBase(const Node* const node)
            : currentNode_{ node }
        {
        }

        value_type operator*() const
        {
            return value_type{
                currentNode_->Index,
                *reinterpret_cast<std::conditional_t<Const, const TData*, TData*>>(currentNode_->Data)
            };
        }

        IteratorBase& operator++()
        {
            currentNode_ = currentNode_->Next;

            return *this;
        }

        bool operator==(const IteratorBase& other) const
        {
            return currentNode_ == other.currentNode_;
        }

        bool operator!=(const IteratorBase& other) const
        {
            return !(*this == other);
        }

    private:
        const Node* currentNode_;
    };

    using Iterator = IteratorBase<false>;

    using ConstIterator = IteratorBase<true>;

    explicit IndexedUnmanagedList(const size_t pageSize = 16384, const size_t dataSize = sizeof(TData))
        : NodeSize{ NodeMetadataSize + dataSize },
          NodesPerPage{ pageSize / NodeSize },
          PageSize{ NodeSize * NodesPerPage },
          BlockInitializerWhenPageAllocated{ TBlockInitializerWhenPageAllocated{} },
          freeHead_{ nullptr },
          allocatedHead_{ nullptr }
    {
    }

    ~IndexedUnmanagedList()
    {
        for (const auto page : pages_)
        {
            operator delete(page);
        }
    }

    IndexedUnmanagedList(const IndexedUnmanagedList&) = delete;

    IndexedUnmanagedList& operator=(const IndexedUnmanagedList&) = delete;

    IndexedUnmanagedList(IndexedUnmanagedList&&) = delete;

    IndexedUnmanagedList& operator=(IndexedUnmanagedList&&) = delete;

    Iterator begin()
    {
        return Iterator{ allocatedHead_ };
    }

    Iterator end()
    {
        return Iterator{ nullptr };
    }

    ConstIterator cbegin() const
    {
        return ConstIterator{ allocatedHead_ };
    }

    ConstIterator cend() const
    {
        return ConstIterator{ nullptr };
    }

    ConstIterator begin() const
    {
        return cbegin();
    }

    ConstIterator end() const
    {
        return cend();
    }

    [[nodiscard]]
    std::pair<uint32_t, TData&> Create()
    {
        if (!freeHead_)
        {
            const auto [headOfNewPage, lastOfNewPage] = ExtendPage();
            freeHead_ = headOfNewPage;
        }

        // ReSharper disable once CppDFANotInitializedField
        const auto returnNode = freeHead_;
        freeHead_ = freeHead_->Next;
        if (freeHead_)
        {
            freeHead_->Prev = nullptr;
        }

        returnNode->Prev = returnNode->Next = nullptr;
        returnNode->Next = allocatedHead_;
        if (allocatedHead_)
        {
            allocatedHead_->Prev = returnNode;
        }
        allocatedHead_ = returnNode;
        returnNode->Allocated = true;

        return { returnNode->Index, *reinterpret_cast<TData*>(returnNode->Data) };
    }

    [[nodiscard]]
    TData* CreateAt(const uint32_t index)
    {
        const auto pageIndex = index / NodesPerPage;
        // ReSharper disable once CppDFALoopConditionNotUpdated
        while (pageIndex >= pages_.size())
        {
            const auto [headOfNewPage, lastOfNewPage] = ExtendPage();
            lastOfNewPage->Next = freeHead_;
            // ReSharper disable once CppDFANotInitializedField
            if (freeHead_)
            {
                freeHead_->Prev = lastOfNewPage;
            }
            freeHead_ = headOfNewPage;
        }

        const auto blockIndex = index % NodesPerPage;
        const auto listNode = reinterpret_cast<Node*>(pages_[pageIndex] + blockIndex * NodeSize);
        if (listNode->Allocated)
        {
            return nullptr;
        }

        // free list head
        if (listNode->Prev)
        {
            listNode->Prev->Next = listNode->Next;
        }
        else
        {
            freeHead_ = listNode->Next;
        }

        if (listNode->Next)
        {
            listNode->Next->Prev = listNode->Prev;
        }

        // allocated list head
        listNode->Next = allocatedHead_;
        if (allocatedHead_)
        {
            allocatedHead_->Prev = listNode;
        }
        allocatedHead_ = listNode;
        listNode->Allocated = true;

        return reinterpret_cast<TData*>(listNode->Data);
    }

    void Erase(const uint32_t index)
    {
        const auto pageIndex = index / NodesPerPage;
        const auto blockIndex = index % NodesPerPage;
        if (pageIndex >= pages_.size())
        {
            return;
        }

        const auto listNode = reinterpret_cast<Node*>(pages_[pageIndex] + blockIndex * NodeSize);
        if (!listNode->Allocated)
        {
            return;
        }

        // allocated list head
        if (listNode->Prev)
        {
            listNode->Prev->Next = listNode->Next;
        }
        else
        {
            allocatedHead_ = listNode->Next;
        }

        if (listNode->Next)
        {
            listNode->Next->Prev = listNode->Prev;
        }

        // free list head
        listNode->Next = freeHead_;
        if (freeHead_)
        {
            freeHead_->Prev = listNode;
        }
        listNode->Allocated = false;
        freeHead_ = listNode;
    }

    [[nodiscard]]
    const TData* Get(const uint32_t index) const
    {
        const auto pageIndex = index / NodesPerPage;
        const auto blockIndex = index % NodesPerPage;
        if (pageIndex >= pages_.size())
        {
            return nullptr;
        }

        const auto listNode = reinterpret_cast<Node*>(pages_[pageIndex] + blockIndex * NodeSize);
        return listNode && listNode->Allocated ? reinterpret_cast<TData*>(listNode->Data) : nullptr;
    }

    [[nodiscard]]
    TData* Get(const uint32_t index)
    {
        return const_cast<TData*>(static_cast<const IndexedUnmanagedList*>(this)->Get(index));
    }

private:
    const size_t NodeSize;
    const size_t NodesPerPage;
    const size_t PageSize;
    const TBlockInitializerWhenPageAllocated BlockInitializerWhenPageAllocated;

    std::vector<char*> pages_;
    Node* freeHead_;
    Node* allocatedHead_;

    std::pair<Node*, Node*> ExtendPage()
    {
        Node* headNodeOfNewPage = nullptr;
        Node* lastNodeOfNewPage = nullptr;
        const auto newPage = static_cast<char*>(operator new(PageSize));
        pages_.push_back(newPage);
        const uint32_t pageBaseIndex = (pages_.size() - 1) * NodesPerPage;

        for (int i = NodesPerPage - 1; i >= 0; --i)
        {
            const auto currentNode = reinterpret_cast<Node*>(newPage + NodeSize * i);
            currentNode->Index = pageBaseIndex + i;
            currentNode->Allocated = false;
            currentNode->Next = headNodeOfNewPage;
            currentNode->Prev = nullptr;

            if (headNodeOfNewPage)
            {
                headNodeOfNewPage->Prev = currentNode;
            }
            headNodeOfNewPage = currentNode;

            if (!lastNodeOfNewPage)
            {
                lastNodeOfNewPage = currentNode;
            }

            BlockInitializerWhenPageAllocated(pageBaseIndex + i, *reinterpret_cast<TData*>(currentNode->Data));
        }

        return { headNodeOfNewPage, lastNodeOfNewPage };
    }
};

#endif //INDEXEDUNMANAGEDLIST_H
