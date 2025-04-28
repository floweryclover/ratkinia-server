#include "NetworkServer.h"
#include "MainServer.h"
#include "GameServer.h"
#include <WinSock2.h>

int main()
{
    SetConsoleOutputCP(65001);

    WSAData wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }

    auto [mainServerSender, mainServerReceiver] = CreateMpscChannel<MainServerPipe>();
    auto [gameServerSender, gameServerReceiver] = CreateMpscChannel<GameServerPipe>();
    auto [networkServerSender, networkServerReceiver] = CreateMpscChannel<NetworkServerPipe>();

    GameServer gameServer{ std::move(gameServerReceiver),
                           mainServerSender,
                           networkServerSender };
    NetworkServer networkServer{ std::move(networkServerReceiver),
                                 std::move(mainServerSender),
                                 std::move(gameServerSender) };
    gameServer.Start();
    networkServer.Start("127.0.0.1", 31415);

    MainServer mainServer{ std::move(mainServerReceiver) };
    mainServer.Run();

    WSACleanup();
    return 0;
}
