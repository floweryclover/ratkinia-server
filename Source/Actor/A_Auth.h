//
// Created by floweryclover on 2025-10-25.
//

#ifndef A_AUTH_H
#define A_AUTH_H

#include "Actor.h"
#include "AuthJob.h"
#include "CtsStub.gen.h"
#include <absl/container/flat_hash_map.h>
#include <queue>

struct Msg_Cts;
struct Msg_SessionDisconnected;

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

    void Handle(std::unique_ptr<Msg_SessionDisconnected> message);

    void OnUnknownMessageType(uint32_t context, RatkiniaProtocol::CtsMessageType messageType) override;

    void OnParseMessageFailed(uint32_t context, RatkiniaProtocol::CtsMessageType messageType) override;

    void OnUnhandledMessageType(RatkiniaProtocol::CtsMessageType messageType) override;

    void OnRegisterRequest(uint32_t context, const std::string& account, const std::string& password) override;

    void OnLoginRequest(uint32_t context, const std::string& account, const std::string& password) override;

protected:
    void Tick() override;

    void OnUnknownMessageReceived(std::unique_ptr<DynamicMessage> message) override
    {
        CRASH_NOW();
        // ReSharper disable once CppDFAUnreachableCode
    }

private:
    absl::flat_hash_map<uint32_t, uint64_t> contextIdMap_;
    absl::flat_hash_map<uint64_t, uint32_t> idContextMap_;

    std::thread authThread_;
    std::atomic_bool shouldAuthThreadStop_;

    std::mutex authJobForegroundQueueMutex_;
    std::queue<AuthJob> authJobForegroundQueue_;

    std::mutex authJobBackgroundQueueMutex_;
    std::queue<AuthJob> authJobBackgroundQueue_;

    [[noreturn]] void AuthBackgroundThreadBody();
};

#endif //A_AUTH_H
