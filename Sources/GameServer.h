//
// Created by floweryclover on 2025-05-01.
//

#ifndef RATKINIASERVER_GAMESERVER_H
#define RATKINIASERVER_GAMESERVER_H

#include "MainServerPipe.h"
#include "NetworkServerPipe.h"
#include "GameServerPipe.h"
#include "Channel.h"
#include "CtsHandler.h"
#include <thread>
#include <memory>

class GameServer final
{
public:
    explicit GameServer(MpscReceiver<GameServerPipe> gameServerReceiver,
                        MpscSender<MainServerPipe> mainServerSender,
                        MpscSender<NetworkServerPipe> networkServerSender);

    ~GameServer();

    void Start();

private:
    MpscReceiver<GameServerPipe> gameServerReceiver_;

    MpscSender<MainServerPipe> mainServerSender_;

    MpscSender<NetworkServerPipe> networkServerSender_;

    CtsHandler ctsHandler_;

    std::thread thread_;

    void ThreadBody();
};


#endif //RATKINIASERVER_GAMESERVER_H
