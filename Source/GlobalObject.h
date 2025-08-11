//
// Created by floweryclover on 2025-08-08.
//

#ifndef GLOBALOBJECT_H
#define GLOBALOBJECT_H

struct GlobalObject
{
    explicit GlobalObject() = default;

    GlobalObject(const GlobalObject&) = delete;

    GlobalObject& operator=(const GlobalObject&) = delete;

    GlobalObject(GlobalObject&&) = delete;

    GlobalObject& operator=(GlobalObject&&) = delete;

    virtual ~GlobalObject() = 0;
};

inline GlobalObject::~GlobalObject()
{
}


#endif // GLOBALOBJECT_H
