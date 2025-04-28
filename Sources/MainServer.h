//
// Created by floweryclover on 2025-04-02.
//

#ifndef MAINSERVER_H
#define MAINSERVER_H

#include "Channel.h"
#include "MainServerPipe.h"

class MainServer final
{
public:
    explicit MainServer(MpscReceiver<MainServerPipe> mainServerReceiver);

    void Run();

private:
    MpscReceiver<MainServerPipe> mainServerReceiver_;
};

#endif //MAINSERVER_H
