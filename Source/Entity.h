//
// Created by floweryclover on 2024-11-25.
// From Staticia Project.
//

#ifndef ENTITY_H
#define ENTITY_H

#include <limits>
#include <cstdint>

struct Entity final
{
    static constexpr size_t VersionBitSize = 12;
    static constexpr size_t IdBitSize = sizeof(uint32_t) * 8 - VersionBitSize;
    static constexpr uint32_t NullId = (1 << IdBitSize) - 1;

    constexpr explicit Entity(const uint32_t id, const uint32_t version)
        : data_{ (version << IdBitSize) | (id & (1 << IdBitSize) - 1)}
    {
    }
    constexpr explicit Entity()
        : data_{std::numeric_limits<uint32_t>::max()}
    {}

    constexpr static Entity NullEntity()
    {
        return Entity{};
    }

    [[nodiscard]]
    uint32_t GetId() const
    {
        return ParseIdOf(this->data_);
    }

    [[nodiscard]]
    uint32_t GetVersion() const
    {
        return ParseVersionOf(this->data_);
    }

    explicit operator uint32_t() const
    {
        return GetId();
    }

    bool operator==(const Entity& other) const = default;

    bool operator!=(const Entity& other) const = default;

    explicit operator bool() const
    {
        return !IsNullEntity();
    }

    bool IsNullEntity() const
    {
        return *this == NullEntity();
    }

private:
    uint32_t data_; // Version | Id

    static uint32_t ParseVersionOf(const uint32_t data)
    {
        return data >> IdBitSize;
    }

    static uint32_t ParseIdOf(const uint32_t data)
    {
        return data & ((1 << IdBitSize) - 1);
    }
};

#endif //ENTITY_H
