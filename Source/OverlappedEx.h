//
// Created by floweryclover on 2025-10-28.
//

#ifndef OVERLAPPEDEX_H
#define OVERLAPPEDEX_H

#include <variant>

using UINT_PTR = unsigned __int64;
using SOCKET = UINT_PTR;

class Session;

struct alignas(64) OverlappedEx final
{
    struct AcceptData final
    {
        char AddressBuffer[64];
        SOCKET ClientSocket;
    };

    struct IoData final
    {
        uint8_t IoType;
        Session* Session;
    };

    char RawOverlapped[32];
    std::variant<AcceptData, IoData> Data;
};

#endif //OVERLAPPEDEX_H
