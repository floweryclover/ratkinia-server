//
// Created by floweryclover on 2025-04-02.
//

#ifndef MAINSERVER_H
#define MAINSERVER_H

#include "Channel.h"
#include "MainServerChannel.h"

class MainServer final
{
public:
    explicit MainServer(MainServerChannel::MpscReceiver mainServerReceiver);

    void Run();

private:
    MainServerChannel::MpscReceiver mainServerReceiver_;
};

#endif //MAINSERVER_H
