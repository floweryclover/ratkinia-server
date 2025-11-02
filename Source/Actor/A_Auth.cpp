//
// Created by floweryclover on 2025-10-25.
//

#include "A_Auth.h"
#include "Msg_Cts.h"
#include <syncstream>
#include <iostream>

#include "ActorNetworkInterface.h"

A_Auth::A_Auth(const ActorInitializer& initializer)
    : Actor{ initializer },
      authThread_{ &A_Auth::AuthBackgroundThreadBody, this }
{
    Accept<Msg_Cts>(this);
}

A_Auth::AuthenticationResult A_Auth::TryAuthenticate(const uint32_t context, const uint64_t id)
{
    if (contextIdMap_.contains(context))
    {
        return AuthenticationResult::DuplicateContext;
    }
    if (idContextMap_.contains(id))
    {
        return AuthenticationResult::IdAlreadyOnline;
    }

    contextIdMap_.emplace(context, id);
    idContextMap_.emplace(id, context);
    return AuthenticationResult::Success;
}

void A_Auth::DeauthenticateByContext(const uint32_t context)
{
    const auto contextIdPair = contextIdMap_.find(context);
    if (contextIdPair == contextIdMap_.end())
    {
        return;
    }

    idContextMap_.erase(contextIdPair->second);
    contextIdMap_.erase(contextIdPair);
}

void A_Auth::DeauthenticateByPlayerId(const uint64_t id)
{
    const auto idContextPair = idContextMap_.find(id);
    if (idContextPair == idContextMap_.end())
    {
        return;
    }

    contextIdMap_.erase(idContextPair->second);
    idContextMap_.erase(idContextPair);
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
}

void A_Auth::OnLoginRequest(const uint32_t context, const std::string& account, const std::string& password)
{
    ActorNetworkInterface.DisconnectSession(context);
}
