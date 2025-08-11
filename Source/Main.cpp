#include "NetworkServer.h"

#include "MainServer.h"
#include "SystemRegistrar.h"
#include "GlobalObjectRegistrar.h"

#include <openssl/err.h>
#include <openssl/ssl.h>
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

    MainServer mainServer
    {
        "127.0.0.1",
        31415,
        "postgresql://ratkinia_agent:1234@127.0.0.1:5432/ratkinia",
        2,
        "ratkinia.crt",
        "ratkinia.key"
    };
    RegisterSystems(mainServer);
    RegisterGlobalObjects(mainServer);

    mainServer.Run();

    return 0;
}
