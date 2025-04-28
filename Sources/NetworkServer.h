//
// Created by floweryclover on 2025-03-31.
//

#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "Channel.h"
#include "GameServerPipe.h"
#include "NetworkServerPipe.h"
#include "MainServerPipe.h"
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
    explicit NetworkServer(MpscReceiver<NetworkServerPipe> networkServerReceiver,
                           MpscSender<MainServerPipe> mainServerSender,
                           MpscSender<GameServerPipe> gameServerSender);

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

    MpscReceiver<NetworkServerPipe> networkServerReceiver_;
    MpscSender<MainServerPipe> mainServerSender_;
    MpscSender<GameServerPipe> gameServerSender_;

    SOCKET listenSocket_;
    HANDLE iocpHandle_;
    std::atomic_bool shouldStop_;

    uint64_t newSessionId_;
    std::unordered_map<uint64_t, Session> sessions_;
    std::unique_ptr<AcceptContext[]> acceptContexts_;

    std::vector<std::thread> workerThreads_;

    void WorkerThreadBody(int threadId);

    bool AcceptAsync();

    void PostAccept(Session& session);

    void PostReceive(Session& session, size_t bytesTransferred);

    void OnMessagePushFailed(size_t sessionId);

    void DisconnectSession(uint64_t session);
};

#endif //NETWORKSERVER_H
