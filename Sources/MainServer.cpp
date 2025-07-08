//
// Created by floweryclover on 2025-04-02.
//

#include "MainServer.h"
#include "MessagePrinter.h"

MainServer::MainServer(MainServerChannel::MpscReceiver mainServerReceiver)
    : mainServerReceiver_{std::move(mainServerReceiver)}
{
}

void MainServer::Run()
{
    while (true)
    {
        mainServerReceiver_.Wait();
        if (mainServerReceiver_.IsClosed())
        {
            MessagePrinter::WriteLine("MainServer Receiver closed");
            break;
        }

        if (const auto message = mainServerReceiver_.TryPeek())
        {
            MessagePrinter::WriteLine("MainServer Receiver Command");
            mainServerReceiver_.Pop(message);
            break;
        }
    }
}