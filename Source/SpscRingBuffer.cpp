//
// Created by floweryclover on 2025-06-26.
//

#include "SpscRingBuffer.h"

SpscRingBuffer::SpscRingBuffer(const size_t capacity, const size_t maxBlockSize)
    : Capacity{ capacity },
      MaxBlockSize{ maxBlockSize },
      ringBuffer_{ std::make_unique<char[]>(capacity) },
      tempDequeueBuffer_{ std::make_unique<char[]>(maxBlockSize) }
{

}
