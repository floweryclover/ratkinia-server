//
// Created by floweryclover on 2025-05-01.
//

#include "GameServer.h"
#include "Database.h"
#include "MainServer.h"
#include "Errors.h"
#include <pqxx/pqxx>

#include "NetworkServer.h"

using namespace Database;
using namespace pqxx;

GameServer::GameServer(GameServerChannel::MpscReceiver gameServerReceiver,
                       MainServerChannel::MpscSender mainServerSender,
                       NetworkServerChannel::SpscSender networkServerSender)
    : gameServerReceiver_{ std::move(gameServerReceiver) },
      mainServerSender_{ std::move(mainServerSender) },
      ctsStub_{ *this },
      stcProxy_{ std::move(networkServerSender) },
      dbConnection_{ DbHost }
{
    dbConnection_.prepare(Prepped_FindUserId, "SELECT * FROM auth.accounts WHERE user_id = $1");
    dbConnection_.prepare(Prepped_InsertAccount, "INSERT INTO auth.accounts (user_id, password) VALUES ($1, $2)");
}

GameServer::~GameServer()
{
    shouldAuthThreadStop_.store(true);
    if (thread_.joinable())
    {
        thread_.join();
    }

    if (authThread_.joinable())
    {
        authThread_.join();
    }
}

void GameServer::Start()
{
    ERR_FAIL_COND(thread_.joinable());

    thread_ = std::thread{ &GameServer::ThreadBody, this };
    authThread_ = std::thread{ &GameServer::AuthThreadBody, this };
}

void GameServer::Terminate()
{
    mainServerSender_.TryPush(std::make_unique<ShutdownCommand>());
}

void GameServer::HandlePostLogin(const uint32_t context, const uint32_t id, const bool isPasswordMatch)
{
    const bool isLoginSuccessful = isPasswordMatch &&
                                   !contextIdMap_.contains(context) &&
                                   !idContextMap_.contains(id);

    stcProxy_.LoginResponse(context, isLoginSuccessful);
}

void GameServer::HandlePostRegister(const uint32_t context, const std::string& id, const std::array<char, 64>& hashedPassword)
{
    {
        nontransaction sameIdSearchWork{dbConnection_};
        if (const auto sameIdSearchResult = sameIdSearchWork.exec(Prepped_FindUserId, id);
            !sameIdSearchResult.empty())
        {
            stcProxy_.RegisterResponse(context, false, "중복된 ID입니다.");
            return;
        }
    }

    work insertAccountWork{dbConnection_};
    const auto insertAccountResult = insertAccountWork.exec(Prepped_InsertAccount, params{id, hashedPassword.data()});
    insertAccountWork.commit();

    const bool isSuccessful = insertAccountResult.affected_rows() == 1;
    CRASH_COND(!isSuccessful);
    stcProxy_.RegisterResponse(context, true, "");
}

void GameServer::ThreadBody()
{
    while (true)
    {
        if (gameServerReceiver_.IsClosed())
        {
            break;
        }

        while (const auto message = gameServerReceiver_.TryPeek())
        {
            ctsStub_.HandleCts(message->Context,
                               message->MessageType,
                               message->BodySize,
                               message->Body);
            gameServerReceiver_.Pop(*message);
        }

        while (true)
        {
            const auto job = [&]() ->const std::unique_ptr<AuthJob<GameServer>>
            {
                std::lock_guard lock{authJobForegroundQueueMutex_};
                if (authJobForegroundQueue_.empty())
                {
                    return nullptr;
                }

                auto front = std::move(authJobForegroundQueue_.front());
                authJobForegroundQueue_.pop();
                return front;
            }();

            if (!job)
            {
                break;
            }

            job->ExecuteFinalForegroundJob(*this);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{ 16 });
    }
}

void GameServer::AuthThreadBody()
{
    MessagePrinter::WriteLine("GameServer Auth Thread 시작");
    while (!shouldAuthThreadStop_.load(std::memory_order_relaxed))
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{ 32 });

            std::unique_ptr<AuthJob<GameServer>> authJob;
            {
                std::lock_guard lock{authJobBackgroundQueueMutex_};
                if (authJobBackgroundQueue_.empty())
                {
                    break;
                }
                authJob = std::move(authJobBackgroundQueue_.front());
                authJobBackgroundQueue_.pop();
            }

            authJob->ExecuteBackgroundJob();

            {
                std::lock_guard lock{authJobForegroundQueueMutex_};
                authJobForegroundQueue_.push(std::move(authJob));
            }
        }
    }
    MessagePrinter::WriteLine("GameServer Auth Thread 종료");
}
