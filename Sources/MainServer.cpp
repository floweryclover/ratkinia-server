//
// Created by floweryclover on 2025-04-02.
//

#include "MainServer.h"
#include "MessagePrinter.h"

MainServer::MainServer(MpscReceiver<MainServerPipe> mainServerReceiver)
    : mainServerReceiver_{std::move(mainServerReceiver)}
{
}

void MainServer::Run()
{
    while (true)
    {
        mainServerReceiver_.Wait();
        if (mainServerReceiver_.Closed())
        {
            MessagePrinter::WriteLine("MainServer Receiver closed");
            break;
        }

        MainServerPipe::Command command;
        if (mainServerReceiver_.TryPeek(command))
        {
            MessagePrinter::WriteLine("MainServer Receiver Command");
            break;
        }
    }
}