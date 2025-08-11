//
// Created by floweryclover on 2025-05-05.
//

#ifndef RATKINIASERVER_CTSSTUB_H
#define RATKINIASERVER_CTSSTUB_H

#include "CtsStub.gen.h"

struct MutableEnvironment;

class CtsStub final : public RatkiniaProtocol::CtsStub<CtsStub>
{
public:
    explicit CtsStub(MutableEnvironment& environment);

    void OnParseMessageFailed(uint32_t context,
                              RatkiniaProtocol::CtsMessageType messageType) override;

    void OnUnknownMessageType(uint32_t context,
                              RatkiniaProtocol::CtsMessageType messageType) override;

    void OnLoginRequest(uint32_t context,
                        const std::string& id,
                        const std::string& password) override;

    void OnRegisterRequest(uint32_t context,
                           const std::string& id,
                           const std::string& password) override;

private:
    MutableEnvironment& environment_;
};

#endif //RATKINIASERVER_CTSSTUB_H
