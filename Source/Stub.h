//
// Created by floweryclover on 2025-05-05.
//

#ifndef RATKINIASERVER_STUB_H
#define RATKINIASERVER_STUB_H

#include "CtsStub.gen.h"

struct MutableEnvironment;

class Stub final : public RatkiniaProtocol::CtsStub<Stub>
{
public:
    explicit Stub(MutableEnvironment& environment);

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

    void OnCreateCharacter(uint32_t context, const std::string& name) override;

    void OnLoadMyCharacters(uint32_t context) override;

    void OnSelectCharacter(uint32_t context, uint64_t id) override;

    void OnDummyRequest(const uint32_t context, const std::string& message) override;

private:
    MutableEnvironment& environment_;
};

#endif //RATKINIASERVER_STUB_H
