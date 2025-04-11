//
// Created by floweryclover on 2025-04-11.
//

#ifndef RATKINIASERVER_MESSAGEPRINTER_H
#define RATKINIASERVER_MESSAGEPRINTER_H

#include <iostream>
#include <mutex>

class MessagePrinter final
{
public:
    template<typename ...Args>
    static void WriteLine(Args...args)
    {
        std::lock_guard<std::mutex> lock{mutex};
        ((std::cout << args << " "), ...) << std::endl;
    }

private:
    inline static std::mutex mutex;
};

#endif //RATKINIASERVER_MESSAGEPRINTER_H
