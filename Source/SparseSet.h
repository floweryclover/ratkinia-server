//
// Created by floweryclover on 2025-08-27.
//

#ifndef SPARSESET_H
#define SPARSESET_H

#include <vector>

class RawSparseSet
{
public:
    virtual ~RawSparseSet() = default;
};

template<typename T>
class SparseSet final : public RawSparseSet
{
public:
    std::vector<std::pair<uint32_t, T>> Data;
};

#endif //SPARSESET_H
