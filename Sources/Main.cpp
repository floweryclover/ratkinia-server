#include "NetworkServer.h"
#include "MainServer.h"
#include "GameServer.h"
#include "GameServerTerminal.h"
#include "NetworkServerTerminal.h"
#include <iostream>
#include <WinSock2.h>

int main()
{
    SetConsoleOutputCP(65001);

    WSAData wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }

    MainServer mainServer;

    NetworkServerTerminal networkServerTerminal;


    GameServer gameServer{ mainServer, gameServer };
    gameServer.Start();

    GameServerTerminal queue;
    NetworkServer networkServer{ mainServer, queue };
    networkServer.Start("127.0.0.1", 31415);

    mainServer.Run();

    WSACleanup();
    return 0;
}
