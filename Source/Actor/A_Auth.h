//
// Created by floweryclover on 2025-10-25.
//

#ifndef A_AUTH_H
#define A_AUTH_H

#include "Actor.h"
#include "AuthJob.h"
#include "CtsStub.gen.h"
#include <absl/container/flat_hash_map.h>
#include <optional>
#include <queue>

struct Msg_Cts;

class A_Auth final : public Actor, public RatkiniaProtocol::ICtsStub<A_Auth>
{
    enum class AuthenticationResult : uint8_t
    {
        Success,
        DuplicateContext,
        IdAlreadyOnline,
    };

public:
    explicit A_Auth(const ActorInitializer& initializer);

    void Handle(std::unique_ptr<Msg_Cts> message);

    void OnUnknownMessageType(uint32_t context, RatkiniaProtocol::CtsMessageType messageType) override;

    void OnParseMessageFailed(uint32_t context, RatkiniaProtocol::CtsMessageType messageType) override;

    void OnUnhandledMessageType(RatkiniaProtocol::CtsMessageType messageType) override;

    void OnRegisterRequest(uint32_t context, const std::string& account, const std::string& password) override;

    void OnLoginRequest(uint32_t context, const std::string& account, const std::string& password) override;

protected:
    void OnUnknownMessageReceived(std::unique_ptr<DynamicMessage> message) override
    {
        CRASH_NOW();
        // ReSharper disable once CppDFAUnreachableCode
    }

private:
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

    absl::flat_hash_map<uint32_t, uint64_t> contextIdMap_;
    absl::flat_hash_map<uint64_t, uint32_t> idContextMap_;

    std::thread authThread_;
    std::atomic_bool shouldAuthThreadStop_;

    std::mutex authJobForegroundQueueMutex_;
    std::queue<AuthJob> authJobForegroundQueue_;

    std::mutex authJobBackgroundQueueMutex_;
    std::queue<AuthJob> authJobBackgroundQueue_;

    void AuthBackgroundThreadBody();
};

#endif //A_AUTH_H
