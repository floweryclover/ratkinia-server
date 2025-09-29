//
// Created by floweryclover on 2025-09-30.
//

#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H

#include "Manager.h"
#include "System.h"
#include <vector>

class SystemManager final : public Manager
{
public:
    template<auto TSystem>
    void RegisterSystem()
    {
        systems_.push_back(TSystem);
    }

    template<auto TSystem>
    void RegisterInitializerSystem()
    {
        initializerSystems_.push_back(TSystem);
    }

    const std::vector<System>& InitializerSystems() const
    {
        return initializerSystems_;
    }

    const std::vector<System>& Systems() const
    {
        return systems_;
    }

private:
    std::vector<System> initializerSystems_;
    std::vector<System> systems_;
};

#endif //SYSTEMMANAGER_H
