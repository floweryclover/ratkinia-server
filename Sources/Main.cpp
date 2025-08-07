#include "NetworkServer.h"
#include "MainServer.h"
#include "GameServer.h"
#include <openssl/err.h>
#include <WinSock2.h>

int main()
{
    SetConsoleOutputCP(65001);

    SSL_load_error_strings();
    ERR_load_crypto_strings();
    WSAData wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }

    auto [mainServerSender, mainServerReceiver] = MainServerChannel::CreateMpscChannel();
    auto [gameServerSender, gameServerReceiver] = GameServerChannel::CreateMpscChannel();
    auto [networkServerSender, networkServerReceiver] = NetworkServerChannel::CreateSpscChannel();

    GameServer gameServer{ std::move(gameServerReceiver),
                           mainServerSender,
                           std::move(networkServerSender) };
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
