//
// Created by floweryclover on 2025-09-30.
//

#ifndef MANAGER_H
#define MANAGER_H

class Manager
{
public:
    explicit Manager() = default;

    virtual ~Manager() = default;

    Manager(const Manager&) = delete;

    Manager& operator=(const Manager&) = delete;

    Manager(Manager&&) = delete;

    Manager& operator=(Manager&&) = delete;
};

#endif //MANAGER_H
