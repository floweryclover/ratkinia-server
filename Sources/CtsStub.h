//
// Created by floweryclover on 2025-05-05.
//

#ifndef RATKINIASERVER_CTSSTUB_H
#define RATKINIASERVER_CTSSTUB_H

#include "CtsStub.gen.h"

class GameServer;

class CtsStub final : public RatkiniaProtocol::CtsStub<CtsStub>
{
public:
    explicit CtsStub(GameServer& gameServer);

    void OnParseMessageFailed(uint64_t context,
                              RatkiniaProtocol::CtsMessageType messageType) override;

    void OnUnknownMessageType(uint64_t context,
                              RatkiniaProtocol::CtsMessageType messageType) override;

    void OnLoginRequest(uint64_t context,
                        const std::string& id,
                        const std::string& hashed_password) override;

    void OnRegisterRequest(uint64_t context,
                           const std::string& id,
                           const std::string& hashed_password) override;

private:
    GameServer& gameServer_;
};

#endif //RATKINIASERVER_CTSSTUB_H
