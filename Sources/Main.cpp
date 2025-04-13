#include "NetworkServer.h"
#include "MainServer.h"
#include <iostream>
#include <WinSock2.h>

using namespace RatkiniaServer;

int main()
{
    SetConsoleOutputCP(65001);

    WSAData wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }

    MainServer mainServer;

    NetworkServer networkServer{ mainServer };
    networkServer.Start("127.0.0.1", 31415);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN  sockaddrIn {};
    inet_pton(AF_INET, "127.0.0.1", &sockaddrIn.sin_addr);
    sockaddrIn.sin_family =AF_INET;
    sockaddrIn.sin_port = htons(31415);

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&sockaddrIn), sizeof (SOCKADDR_IN)))
    {
        std::cout << "에러: " << WSAGetLastError() << std::endl;
    }
    else
    {
        std::cout << "성공!" << std::endl;
    }

    mainServer.Run();

    WSACleanup();
    return 0;
}
