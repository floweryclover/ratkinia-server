//
// Created by floweryclover on 2025-06-13.
//

#ifndef RATKINIASERVER_SCOPEDNETWORKMESSAGE_H
#define RATKINIASERVER_SCOPEDNETWORKMESSAGE_H

#include <memory>
#include <cstdint>

template<typename TOwner>
class ScopedNetworkMessage final
{
public:
    explicit ScopedNetworkMessage(TOwner& owner,
                                  const uint64_t context,
                                  const uint16_t messageType,
                                  const uint16_t bodySize,
                                  const char* const body)
        : Context{ context },
          MessageType{ messageType },
          BodySize{ bodySize },
          Body{ body },
          owner_{ owner }
    {

    }

    ~ScopedNetworkMessage()
    {
        owner_.ReleaseScopedMessage(*this);
    }

    ScopedNetworkMessage(const ScopedNetworkMessage&) = delete;

    ScopedNetworkMessage& operator=(const ScopedNetworkMessage&) = delete;

    ScopedNetworkMessage(ScopedNetworkMessage&&) = delete;

    ScopedNetworkMessage& operator=(ScopedNetworkMessage&&) = delete;

    const uint64_t Context;
    const uint16_t MessageType;
    const uint16_t BodySize;
    const char* const Body;

private:
    TOwner& owner_;
};


#endif //RATKINIASERVER_SCOPEDNETWORKMESSAGE_H
