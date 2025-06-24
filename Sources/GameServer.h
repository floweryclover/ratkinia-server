//
// Created by floweryclover on 2025-05-01.
//

#ifndef RATKINIASERVER_GAMESERVER_H
#define RATKINIASERVER_GAMESERVER_H

#include "MainServerChannel.h"
#include "NetworkServerChannel.h"
#include "GameServerChannel.h"
#include "Channel.h"
#include "CtsStub.h"
#include "StcProxy.h"
#include <thread>
#include <memory>
#include <pqxx/pqxx>

class GameServer final
{
public:
    static constexpr const char* DbHost = "postgresql://ratkinia_agent:1234@127.0.0.1:5432/ratkinia";

    explicit GameServer(GameServerChannel::MpscReceiver gameServerReceiver,
                        MainServerChannel::MpscSender mainServerSender,
                        NetworkServerChannel::SpscSender networkServerSender);

    ~GameServer();

    void Start();

    void ShouldTerminate();

    [[nodiscard]]
    pqxx::connection& GetDbConnection()
    {
        return dbConnection_;
    }

    [[nodiscard]]
    __forceinline StcProxy& GetStcProxy()
    {
        return stcProxy_;
    }

private:
    GameServerChannel::MpscReceiver gameServerReceiver_;

    MainServerChannel::MpscSender mainServerSender_;

    CtsStub ctsStub_;

    StcProxy stcProxy_;

    std::thread thread_;

    pqxx::connection dbConnection_;

    void ThreadBody();
};


#endif //RATKINIASERVER_GAMESERVER_H
