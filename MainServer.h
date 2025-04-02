//
// Created by floweryclover on 2025-04-02.
//

#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <string>
#include <atomic>

namespace RatkiniaServer
{
    class MainServer final
    {
    public:
        explicit MainServer();

        const std::string& Run();

        void RequestTerminate(int code, std::string reason);

    private:
        std::atomic_bool shouldTerminate_;
        std::string terminateReason_;
    };
}

#endif //MAINSERVER_H
