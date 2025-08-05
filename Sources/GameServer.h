//
// Created by floweryclover on 2025-05-01.
//

#ifndef RATKINIASERVER_GAMESERVER_H
#define RATKINIASERVER_GAMESERVER_H

#include "AuthJob.h"
#include "MainServerChannel.h"
#include "NetworkServerChannel.h"
#include "GameServerChannel.h"
#include "CtsStub.h"
#include "StcProxy.h"
#include <thread>
#include <unordered_map>
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

    void Terminate();

    void EnqueueAuthJob(std::unique_ptr<AuthJob<GameServer>> authJob)
    {
        std::lock_guard lock{authJobBackgroundQueueMutex_};
        authJobBackgroundQueue_.push(std::move(authJob));
    }

    [[nodiscard]]
    pqxx::connection& GetDbConnection()
    {
        CRASH_COND(!dbConnection_.is_open());
        return dbConnection_;
    }

    [[nodiscard]]
    StcProxy& GetStcProxy()
    {
        return stcProxy_;
    }

    void HandlePostLogin(uint32_t context, uint32_t id, bool isPasswordMatch);

    void HandlePostRegister(uint32_t context, const std::string& id, const std::array<char, 64>& hashedPassword);

private:
    GameServerChannel::MpscReceiver gameServerReceiver_;
    MainServerChannel::MpscSender mainServerSender_;
    CtsStub ctsStub_;
    StcProxy stcProxy_;
    std::thread thread_;
    pqxx::connection dbConnection_;

    std::thread authThread_;
    std::atomic_bool shouldAuthThreadStop_;
    std::mutex authJobForegroundQueueMutex_;
    std::mutex authJobBackgroundQueueMutex_;
    std::queue<std::unique_ptr<AuthJob<GameServer>>> authJobForegroundQueue_;
    std::queue<std::unique_ptr<AuthJob<GameServer>>> authJobBackgroundQueue_;

    std::unordered_map<uint32_t, uint32_t> contextIdMap_;
    std::unordered_map<uint32_t, uint32_t> idContextMap_;

    void ThreadBody();

    void AuthThreadBody();
};


#endif //RATKINIASERVER_GAMESERVER_H
