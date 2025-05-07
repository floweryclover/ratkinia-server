//
// Created by floweryclover on 2025-05-05.
//

#ifndef RATKINIASERVER_CTSHANDLER_H
#define RATKINIASERVER_CTSHANDLER_H

#include "CtsStub.gen.h"
#include <functional>

class GameServer;

class CtsHandler final : public RatkiniaProtocol::CtsStub<CtsHandler>
{
public:
    explicit CtsHandler(GameServer& gameServer);

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

#endif //RATKINIASERVER_CTSHANDLER_H
