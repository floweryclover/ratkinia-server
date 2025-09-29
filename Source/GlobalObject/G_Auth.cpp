//
// Created by floweryclover on 2025-08-08.
//

#include "G_Auth.h"
#include "MessagePrinter.h"

G_Auth::G_Auth()
    : authThread_{ &G_Auth::AuthBackgroundThreadBody, this }
{
}

G_Auth::AuthenticationResult G_Auth::TryAuthenticate(const uint32_t context, const uint64_t id)
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

void G_Auth::DeauthenticateByContext(const uint32_t context)
{
    const auto contextIdPair = contextIdMap_.find(context);
    if (contextIdPair == contextIdMap_.end())
    {
        return;
    }

    idContextMap_.erase(contextIdPair->second);
    contextIdMap_.erase(contextIdPair);
}

void G_Auth::DeauthenticateByPlayerId(const uint64_t id)
{
    const auto idContextPair = idContextMap_.find(id);
    if (idContextPair == idContextMap_.end())
    {
        return;
    }

    contextIdMap_.erase(idContextPair->second);
    idContextMap_.erase(idContextPair);
}

void G_Auth::AuthBackgroundThreadBody()
{
    MessagePrinter::WriteLine("G_Auth 백그라운드 스레드 시작");
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
    MessagePrinter::WriteLine("G_Auth 백그라운드 스레드 종료");
}
