#include "NetworkServer.h"
#include "MainServer.h"
#include <iostream>
#include <WinSock2.h>
#include <cstdint>

using namespace RatkiniaServer;

int main()
{
    int8_t a;
    SetConsoleOutputCP(65001);

    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }

    MainServer mainServer;

    NetworkServer networkServer{ mainServer};
    networkServer.Start("127.0.0.1", 1);

    std::cout << mainServer.Run() << std::endl;

    WSACleanup();
    return 0;
}
