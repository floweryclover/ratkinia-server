//
// Created by floweryclover on 2025-03-31.
//

#include "NetworkServer.h"
#include "MainServer.h"
#include <format>

using namespace RatkiniaServer;

NetworkServer::NetworkServer(MainServer& mainServer)
    : mainServer_{mainServer},
      listenSocket_{INVALID_SOCKET}
{
}

void NetworkServer::Start(const std::string& listenAddress, unsigned short listenPort)
{
    if (listenThread_.joinable())
    {
        mainServer_.RequestTerminate(1, "NetworkServer::Start()를 중복 호출하였습니다.");
        return;
    }

    char addrBuf[INET_ADDRSTRLEN];
    if (inet_pton(AF_INET, listenAddress.c_str(), addrBuf) != 1)
    {
        mainServer_.RequestTerminate(1, std::format("IP 문자열 해석에 실패하였습니다: {}", listenAddress));
        return;
    }

    listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket_ == INVALID_SOCKET)
    {
        mainServer_.RequestTerminate(1, "NetworkServer의 리슨 소켓 생성에 실패하였습니다.");
        return;
    }
}
