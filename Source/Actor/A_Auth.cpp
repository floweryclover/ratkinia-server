//
// Created by floweryclover on 2025-10-25.
//

#include "A_Auth.h"
#include "Msg_Cts.h"
#include "Msg_SessionDisconnected.h"
#include "ActorNetworkInterface.h"
#include "DbConnectionPool.h"
#include "DbConnection.h"
#include <syncstream>
#include <iostream>
#include <regex>

#include "DbService_Account.h"

constexpr const char* FailedReason[] =
{
    "",
    "중복된 아이디입니다.",
    "알 수 없는 에러가 발생하였습니다."
};

A_Auth::A_Auth(const ActorInitializer& initializer)
    : Actor{ initializer },
      authThread_{ &A_Auth::AuthBackgroundThreadBody, this }
{
    Accept<Msg_Cts>(this);
    Accept<Msg_SessionDisconnected>(this);
}

void A_Auth::AuthBackgroundThreadBody()
{
    std::osyncstream{ std::cout } << "A_Auth 백그라운드 스레드 시작" << std::endl;

    while (!shouldAuthThreadStop_.load(std::memory_order_relaxed))
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{ 32 });

            auto authJob = [&]() -> std::optional<AuthJob>
            {
                std::lock_guard lock{ authJobBackgroundQueueMutex_ };
                if (authJobBackgroundQueue_.empty())
                {
                    return std::nullopt;
                }
                auto front = std::move(authJobBackgroundQueue_.front());
                authJobBackgroundQueue_.pop();
                return front;
            }();

            if (!authJob)
            {
                continue;
            }

            if (std::holds_alternative<LoginJob>(*authJob))
            {
                std::get<LoginJob>(*authJob).ExecuteBackgroundJob();
            }
            else
            {
                std::get<RegisterJob>(*authJob).ExecuteBackgroundJob();
            }

            {
                std::lock_guard lock{ authJobForegroundQueueMutex_ };
                authJobForegroundQueue_.push(std::move(*authJob));
            }
        }
    }

    std::osyncstream{ std::cout } << "A_Auth 백그라운드 스레드 종료" << std::endl;
}

void A_Auth::Handle(std::unique_ptr<Msg_Cts> message)
{
    HandleCts(message->Context, message->MessageType, message->BodySize, message->Body.get());
}

void A_Auth::Handle(std::unique_ptr<Msg_SessionDisconnected> message)
{
    const auto contextIdIter = contextIdMap_.find(message->Context);
    if (contextIdIter == contextIdMap_.end())
    {
        return;
    }

    idContextMap_.erase(contextIdIter->second);
    contextIdMap_.erase(contextIdIter);
}

void A_Auth::OnUnknownMessageType(const uint32_t context, const RatkiniaProtocol::CtsMessageType messageType)
{
}

void A_Auth::OnParseMessageFailed(const uint32_t context, const RatkiniaProtocol::CtsMessageType messageType)
{
}

void A_Auth::OnUnhandledMessageType(const RatkiniaProtocol::CtsMessageType messageType)
{
}

void A_Auth::OnRegisterRequest(const uint32_t context, const std::string& account, const std::string& password)
{
    if (account.length() < 6)
    {
        Network.RegisterResponse(context, false, "아이디가 너무 짧습니다.");
        return;
    }
    if (account.length() > 24)
    {
        Network.RegisterResponse(context, false, "아이디가 너무 깁니다.");
        return;
    }
    if (const std::regex invalidAccountRegex{ R"([^a-z0-9_])" };
        std::regex_search(account, invalidAccountRegex))
    {
        Network.RegisterResponse(context, false, "아이디는 영문 소문자, 숫자 또는 언더스코어(_)로만 구성되어야 합니다.");
        return;
    }


    if (password.length() < 8)
    {
        Network.RegisterResponse(context, false, "패스워드가 너무 짧습니다.");
        return;
    }
    if (password.length() > 32)
    {
        Network.RegisterResponse(context, false, "패스워드가 너무 깁니다.");
        return;
    }
    if (const std::regex invalidPasswordRegex{ R"([^a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?])" };
        std::regex_search(password, invalidPasswordRegex))
    {
        Network.RegisterResponse(context, false, "패스워드에 허용되지 않는 문자가 존재합니다.");
        return;
    }

    {
        std::scoped_lock lock{ authJobBackgroundQueueMutex_ };
        authJobBackgroundQueue_.emplace(std::in_place_type<RegisterJob>, context, account, password);
    }
}

void A_Auth::OnLoginRequest(const uint32_t context, const std::string& account, const std::string& password)
{
    if (contextIdMap_.contains(context))
    {
        Network.Notify(context, RatkiniaProtocol::Notify_Type_Fatal, "이미 로그인되어 있는 상태입니다.");
        Network.DisconnectSession(context);
        return;
    }

    if (auto dbConnection = DbConnectionPool.Acquire();
        const auto pair = dbConnection.Access<DbService_Account>().GetAccountIdPasswordPair(account))
    {
        std::lock_guard lock{ authJobBackgroundQueueMutex_ };
        authJobBackgroundQueue_.emplace(std::in_place_type<LoginJob>,
                                        context,
                                        pair->first,
                                        password,
                                        std::move(pair->second));
    }
    else
    {
        std::lock_guard lock{ authJobBackgroundQueueMutex_ };
        authJobBackgroundQueue_.emplace(std::in_place_type<LoginJob>,
                                        context,
                                        LoginJob::InvalidId,
                                        password,
                                        EmptyHashedPassword);
    }
}

void A_Auth::Tick()
{
    while (true)
    {
        const auto job = [&]() -> std::optional<AuthJob>
        {
            std::scoped_lock lock{ authJobForegroundQueueMutex_ };

            if (authJobForegroundQueue_.empty())
            {
                return std::nullopt;
            }

            auto returnValue = std::move(authJobForegroundQueue_.front());
            authJobForegroundQueue_.pop();
            return std::move(returnValue);
        }();

        if (!job)
        {
            break;
        }

        if (std::holds_alternative<LoginJob>(*job))
        {
            auto& loginJob = std::get<LoginJob>(*job);

            if (!loginJob.IsPasswordMatch())
            {
                Network.LoginResponse(loginJob.Context, RatkiniaProtocol::LoginResponse_LoginResult_Failure);
                return;
            }

            const auto contextAddResult = [&]
            {
                if (contextIdMap_.contains(loginJob.Context))
                {
                    return AuthenticationResult::DuplicateContext;
                }
                if (idContextMap_.contains(loginJob.Id))
                {
                    return AuthenticationResult::IdAlreadyOnline;
                }

                contextIdMap_.emplace(loginJob.Context, loginJob.Id);
                idContextMap_.emplace(loginJob.Id, loginJob.Context);
                return AuthenticationResult::Success;
            }();

            if (contextAddResult == AuthenticationResult::Success)
            {
                Network.LoginResponse(loginJob.Context, RatkiniaProtocol::LoginResponse_LoginResult_Success);
            }
            else if (contextAddResult == AuthenticationResult::IdAlreadyOnline)
            {
                Network.LoginResponse(loginJob.Context,
                                      RatkiniaProtocol::LoginResponse_LoginResult_DuplicateAccount);
            }
            else
            {
                Network.LoginResponse(loginJob.Context,
                                      RatkiniaProtocol::LoginResponse_LoginResult_DuplicateContext);
            }
        }
        else
        {
            auto& registerJob = std::get<RegisterJob>(*job);

            auto dbConnection = DbConnectionPool.Acquire();
            const auto result =
                dbConnection
                .Access<DbService_Account>()
                .TryCreateAccount(registerJob.Id,
                                  registerJob.GetHashedPassword());

            if (result != DbService_Account::CreateAccountResult::Success)
            {
                Network.RegisterResponse(registerJob.Context, false, FailedReason[static_cast<int>(result)]);
                return;
            }

            Network.RegisterResponse(registerJob.Context, true, "");
        }
    }
}
