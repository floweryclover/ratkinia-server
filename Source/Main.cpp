#include "MainServer.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <WinSock2.h>
#include <mimalloc-new-delete.h>

// ReSharper disable once CppDFAConstantFunctionResult
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

    MainServer* mainServer = new MainServer
    {
        std::thread::hardware_concurrency(),
        "127.0.0.1",
        31415,
        "postgresql://ratkinia_agent:1234@127.0.0.1:5432/ratkinia",
        16,
        2,
        "ratkinia.crt",
        "ratkinia.key"
    };

    std::thread commandInputThread
    {
        [&mainServer]
        {
            // ReSharper disable once CppDFAEndlessLoop
            while (true)
            {
                std::string command;
                std::cin >> command;

                delete mainServer;
                mainServer->AddCommand(std::move(command));
            }
        }
    };

    try
    {
        mainServer->Run();
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        std::abort();
    }

    // ReSharper disable once CppDFAUnreachableCode
    return 0;
}
