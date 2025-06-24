//
// Created by floweryclover on 2025-03-31.
//

#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "Channel.h"
#include "GameServerChannel.h"
#include "NetworkServerChannel.h"
#include "MainServerChannel.h"
#include "Session.h"
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <thread>
#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <unordered_map>

class NetworkServer final
{
public:
    explicit NetworkServer(NetworkServerChannel::SpscReceiver networkServerReceiver,
                           MainServerChannel::MpscSender mainServerSender,
                           GameServerChannel::MpscSender gameServerSender);

    ~NetworkServer();

    void Start(const std::string& listenAddress, unsigned short listenPort);


private:
    static constexpr uint64_t NullSessionId = 0xffffffffffffffff;
    struct AcceptContext final
    {
        OverlappedEx Context{};
        uint64_t SessionId{};
    };

    static constexpr int AcceptPoolSize = 1;
    static constexpr size_t SessionBufferSize = 1024;

    NetworkServerChannel::SpscReceiver networkServerReceiver_;
    MainServerChannel::MpscSender mainServerSender_;
    GameServerChannel::MpscSender gameServerSender_;

    SOCKET listenSocket_;
    HANDLE iocpHandle_;
    std::atomic_bool shouldStop_;

    uint64_t newSessionId_;
    std::unordered_map<uint64_t, Session> sessions_;
    std::mutex sessionsMutex_;
    std::unique_ptr<AcceptContext[]> acceptContexts_;

    std::vector<std::thread> workerThreads_;
    std::thread channelReceiverThread_;

    void WorkerThreadBody(int threadId);

    void ChannelReceiverThreadBody();

    void AcceptAsync();

    void PostAccept(Session& session);

    void PostReceive(Session& session, size_t bytesTransferred);

    void PostSend(Session& session, size_t bytesTransferred);

    void DisconnectSession(uint64_t sessionId);
};

#endif //NETWORKSERVER_H
