//
// Created by floweryclover on 2025-08-27.
//

#ifndef REGISTRAR_H
#define REGISTRAR_H

#include "ComponentManager.h"
#include "System.h"
#include "EventQueue.h"
#include "GlobalObject.h"
#include <algorithm>

class Registrar final
{
public:
    explicit Registrar()
    {
        sparseSets_.emplace_back();
    }

    ~Registrar() = default;

    Registrar(const Registrar&) = delete;

    Registrar& operator=(const Registrar&) = delete;

    Registrar(Registrar&&) = default;

    Registrar& operator=(Registrar&&) = default;

    template<typename TComponent>
    void RegisterComponent()
    {
        const uint32_t runtimeOrder = TComponent::GetRuntimeOrder();
        auto& sparseSets = sparseSets_;

        if (sparseSets.size() <= runtimeOrder)
        {
            sparseSets.resize(runtimeOrder + 1);
        }

        CRASH_COND(sparseSets[runtimeOrder] != nullptr);
        sparseSets[runtimeOrder] = std::make_unique<SparseSet<TComponent>>();
    }

    void RegisterSystem(const System system)
    {
        const bool isSystemAlreadyRegistered = std::ranges::any_of(systems_,
                                                                   [system](const System registeredSystem)
                                                                   {
                                                                       return system == registeredSystem;
                                                                   });
        CRASH_COND(isSystemAlreadyRegistered);
        systems_.emplace_back(system);
    }

    template<typename TEvent>
    void RegisterEvent()
    {
        TEvent::SetRuntimeOrder(eventQueues_.size());
        eventQueues_.emplace_back(std::make_unique<EventQueue<TEvent>>());
    }

    template<typename TGlobalObject, typename... Args>
    void RegisterGlobalObject(Args&&... args)
    {
        const uint32_t runtimeOrder = globalObjects_.size();
        TGlobalObject::SetRuntimeOrder(runtimeOrder);
        globalObjects_.emplace_back(std::make_unique<TGlobalObject>(std::forward<Args>(args)...));
    }

    void RegisterInitializerSystem(const System system)
    {
        const bool isInitializerSystemAlreadyRegistered = std::ranges::any_of(initializerSystems_,
                                                                   [system](const System registeredSystem)
                                                                   {
                                                                       return system == registeredSystem;
                                                                   });
        CRASH_COND(isInitializerSystemAlreadyRegistered);
        initializerSystems_.emplace_back(system);
    }

    std::vector<std::unique_ptr<RawSparseSet>> GetSparseSets()
    {
        return std::move(sparseSets_);
    }

    std::vector<System> GetSystems()
    {
        return std::move(systems_);
    }

    std::vector<std::unique_ptr<RawEventQueue>> GetEventQueues()
    {
        return std::move(eventQueues_);
    }

    std::vector<std::unique_ptr<GlobalObject>> GetGlobalObjects()
    {
        return std::move(globalObjects_);
    }

    std::vector<System> GetInitializerSystems()
    {
        return std::move(initializerSystems_);
    }

private:
    std::vector<std::unique_ptr<RawSparseSet>> sparseSets_;
    std::vector<System> systems_;
    std::vector<std::unique_ptr<RawEventQueue>> eventQueues_;
    std::vector<std::unique_ptr<GlobalObject>> globalObjects_;
    std::vector<System> initializerSystems_;
};

#endif //REGISTRAR_H
