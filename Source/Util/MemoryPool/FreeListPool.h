//
// Created by floweryclover on 2025-07-18.
// From Staticia Project.
//

#ifndef LISTPOOL_H
#define LISTPOOL_H

#include <vector>

#pragma pack(push, 1)

#pragma pack(pop)

/**
 * Free List 방식으로 동작하는 메모리풀. 기본적으로는 한 페이지 64KB, T 크기만큼의 블록으로 설정되지만 별도 설정 가능.
 * @remarks - T에 대한 어떠한 생성자, 소멸자 호출도 없으므로 사용자 코드에 책임이 있음.
 * @tparam T
 */
template<typename T>
class FreeListPool final
{
#pragma pack(push, 1)
    struct ListNode
    {
        ListNode* Next;
        char Data[];
    };
#pragma pack(pop)

public:
    static constexpr size_t NodeMetadataSize = offsetof(ListNode, Data);

    explicit FreeListPool(const size_t dataSize = sizeof(T),
                          const size_t blocksPerPage = 65536 / (NodeMetadataSize + sizeof(T)))
        : DataSize{ dataSize },
          BlocksPerPage{ blocksPerPage },
          BlockSize{ NodeMetadataSize + dataSize },
          PageSize{ BlockSize * blocksPerPage },
          head_{ nullptr }
    {
    }

    ~FreeListPool()
    {
        for (const auto page : pages_)
        {
            operator delete(page);
        }
    }

    FreeListPool(const FreeListPool&) = delete;

    FreeListPool& operator=(const FreeListPool&) = delete;

    FreeListPool(FreeListPool&&) = delete;

    FreeListPool& operator=(FreeListPool&&) = delete;

    [[nodiscard]]
    T* Acquire()
    {
        if (!head_)
        {
            ListNode* nextNode = nullptr;
            const auto newPage = static_cast<char*>(operator new(PageSize));
            pages_.push_back(newPage);

            for (int i = BlocksPerPage - 1; i >= 0; --i)
            {
                const auto currentNode = reinterpret_cast<ListNode*>(newPage + BlockSize * i);
                currentNode->Next = nextNode;
                nextNode = currentNode;
            }

            head_ = nextNode;
        }

        const auto returnNode = head_;
        head_ = head_->Next;
        returnNode->Next = nullptr;
        return reinterpret_cast<T*>(returnNode->Data);
    }

    void Release(T* const data)
    {
        const auto node = reinterpret_cast<ListNode*>(reinterpret_cast<char*>(data) - offsetof(ListNode, Data));
        SCRASH_COND(node->Next != nullptr);

        node->Next = head_;
        head_ = node;
    }

private:
    const size_t DataSize;
    const size_t BlocksPerPage;
    const size_t BlockSize;
    const size_t PageSize;

    std::vector<char*> pages_;
    ListNode* head_;
};

#endif //LISTPOOL_H
