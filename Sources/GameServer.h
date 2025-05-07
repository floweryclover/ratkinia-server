//
// Created by floweryclover on 2025-05-01.
//

#ifndef RATKINIASERVER_GAMESERVER_H
#define RATKINIASERVER_GAMESERVER_H

#include "CtsHandler.h"
#include "GameServerTerminal.h"
#include <thread>
#include <memory>

class MainServer;

class CtsHandler;

class GameServer final
{
public:
    explicit GameServer(MainServer& mainServer);

    ~GameServer();

    void Start();

    void DisconnectSession(uint64_t session);

    void Terminate();

    __forceinline bool PushMessage(uint64_t session, uint16_t messageType, uint16_t bodySize, const char* body)
    {
        return messageQueue_.PushMessage(session, messageType, bodySize, body);
    }

private:
    MainServer& mainServer_;

    GameServerTerminal messageQueue_;

    CtsHandler ctsHandler_;

    std::thread thread_;

    void ThreadBody();
};


#endif //RATKINIASERVER_GAMESERVER_H
