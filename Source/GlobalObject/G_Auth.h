//
// Created by floweryclover on 2025-08-08.
//

#ifndef G_AUTH_H
#define G_AUTH_H

#include "AuthJob.h"
#include "GlobalObject.h"
#include <unordered_map>
#include <optional>
#include <thread>
#include <mutex>
#include <queue>

struct G_Auth final : GlobalObject
{
    GLOBALOBJECT(G_Auth)

    enum class AuthenticationResult : uint8_t
    {
        Success,
        DuplicateContext,
        IdAlreadyOnline,
    };

    explicit G_Auth();

    AuthenticationResult TryAuthenticate(uint32_t context, uint64_t id);

    void DeauthenticateByContext(uint32_t context);

    void DeauthenticateByPlayerId(uint64_t id);

    std::optional<AuthJob> TryPopFinishedBackgroundJob()
    {
        std::lock_guard lock{authJobForegroundQueueMutex_};

        if (authJobForegroundQueue_.empty())
        {
            return std::nullopt;
        }

        auto returnValue = std::move(authJobForegroundQueue_.front());
        authJobForegroundQueue_.pop();

        return std::make_optional(std::move(returnValue));
    }

    std::optional<uint64_t> TryGetPlayerIdOfContext(const uint32_t context)
    {
        return contextIdMap_.contains(context) ? std::make_optional(contextIdMap_.at(context)) : std::nullopt;
    }

    std::optional<uint32_t> TryGetContextOfPlayerId(const uint64_t id)
    {
        return idContextMap_.contains(id) ? std::make_optional(idContextMap_.at(id)) : std::nullopt;
    }

    template<typename TJob, typename ...Args>
    void CreateAuthJob(Args&&... args)
    {
        std::lock_guard lock{authJobBackgroundQueueMutex_};
        authJobBackgroundQueue_.emplace(TJob{std::forward<Args>(args)...});
    }

private:
    std::unordered_map<uint32_t, uint64_t> contextIdMap_;
    std::unordered_map<uint64_t, uint32_t> idContextMap_;

    std::thread authThread_;
    std::atomic_bool shouldAuthThreadStop_;
    std::mutex authJobForegroundQueueMutex_;
    std::mutex authJobBackgroundQueueMutex_;

    std::queue<AuthJob> authJobForegroundQueue_;
    std::queue<AuthJob> authJobBackgroundQueue_;

    void AuthBackgroundThreadBody();
};

#endif //G_AUTH_H
