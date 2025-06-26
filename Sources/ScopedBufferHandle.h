//
// Created by floweryclover on 2025-06-26.
//

#ifndef RATKINIASERVER_SCOPEDBUFFERHANDLE_H
#define RATKINIASERVER_SCOPEDBUFFERHANDLE_H

struct ReleaseEnqueueBufferPolicy final
{
    using BufferType = char*;

    template<typename TOwnerBuffer>
    static void Release(TOwnerBuffer* const owner, char* const buffer, const size_t bufferSize)
    {
        if (owner)
        {
            owner->ReleaseEnqueueBuffer(buffer, bufferSize);
        }
    }
};

struct ReleaseDequeueBufferPolicy final
{
    using BufferType = const char*;

    template<typename TOwnerBuffer>
    static void Release(TOwnerBuffer* const owner, const char* const buffer, const size_t bufferSize)
    {
        if (owner)
        {
            owner->ReleaseDequeueBuffer(buffer, bufferSize);
        }
    }
};

template<typename TOwnerBuffer, typename TReleasePolicy>
class ScopedBufferHandle final
{
public:
    const TReleasePolicy::BufferType Buffer;
    const size_t BufferSize;

    __forceinline explicit ScopedBufferHandle(TOwnerBuffer* owner, const TReleasePolicy::BufferType buffer, const size_t bufferSize)
        : Buffer{ buffer },
          BufferSize{ bufferSize },
          Owner{ owner }
    {
    }

    __forceinline ~ScopedBufferHandle()
    {
        TReleasePolicy::Release(Owner, Buffer, BufferSize);
    }

    ScopedBufferHandle(const ScopedBufferHandle&) = delete;

    ScopedBufferHandle& operator=(const ScopedBufferHandle&) = delete;

    ScopedBufferHandle(ScopedBufferHandle&& rhs) = delete;

    ScopedBufferHandle& operator=(ScopedBufferHandle&& rhs) = delete;

private:
    TOwnerBuffer* const Owner;
};

template<typename TOwnerBuffer>
using ScopedBufferDequeuer = ScopedBufferHandle<TOwnerBuffer, ReleaseDequeueBufferPolicy>;

template<typename TOwnerBuffer>
using ScopedBufferEnqueuer = ScopedBufferHandle<TOwnerBuffer, ReleaseEnqueueBufferPolicy>;

#endif //RATKINIASERVER_SCOPEDBUFFERHANDLE_H
