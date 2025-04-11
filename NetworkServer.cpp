//
// Created by floweryclover on 2025-03-31.
//

#include "NetworkServer.h"
#include "MainServer.h"
#include "MessagePrinter.h"
#include <format>

using namespace RatkiniaServer;

NetworkServer::NetworkServer(MainServer& mainServer)
    : mainServer_{ mainServer },
      listenSocket_{ INVALID_SOCKET }
{
}

NetworkServer::~NetworkServer()
{
    if (listenSocket_ != INVALID_SOCKET)
    {
        closesocket(listenSocket_);
    }
}

void NetworkServer::Start(const std::string& listenAddress, unsigned short listenPort)
{
    if (listenThread_.joinable())
    {
        mainServer_.RequestTerminate(1, "NetworkServer::Start()를 중복 호출하였습니다.");
        return;
    }

    SOCKADDR_IN sockaddrIn{};
    sockaddrIn.sin_port = htons(listenPort);
    sockaddrIn.sin_family = AF_INET;
    if (inet_pton(AF_INET, listenAddress.c_str(), &sockaddrIn.sin_addr) != 1)
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

    if (bind(listenSocket_, reinterpret_cast<sockaddr*>(&sockaddrIn), sizeof(SOCKADDR_IN)))
    {
        mainServer_.RequestTerminate(WSAGetLastError(), "bind() failed");
        return;
    }

    const auto iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (iocpHandle == nullptr)
    {
        mainServer_.RequestTerminate(static_cast<int>(GetLastError()), "CreateIoCompletionPort() failed");
        return;
    }

    const auto threadCount = std::thread::hardware_concurrency();
    for (int i=0; i<threadCount; ++i)
    {
        workerThreads_.emplace_back(&NetworkServer::WorkerThreadBody, this, i);
    }
}

void NetworkServer::WorkerThreadBody(const int threadId)
{
    MessagePrinter::WriteLine("IOCP 워커 스레드", threadId, "시작");
}

void NetworkServer::ListenThreadBody()
{

}
