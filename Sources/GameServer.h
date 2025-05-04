//
// Created by floweryclover on 2025-05-01.
//

#ifndef RATKINIASERVER_GAMESERVER_H
#define RATKINIASERVER_GAMESERVER_H

#include <thread>

class MpscMessageQueue;

class GameServer final
{
    static constexpr auto A = [](){};
public:
    explicit GameServer(MpscMessageQueue& messageQueue);

    void Start();

private:
    MpscMessageQueue& messageQueue_;

    std::thread thread_;

    void ThreadBody();

    void HandleMessage(uint64_t sessionId, uint16_t messageType, uint16_t bodySize, const char* body);
};


#endif //RATKINIASERVER_GAMESERVER_H
