#ifndef ENTITYSYSTEM_H_INCLUDED
#define ENTITYSYSTEM_H_INCLUDED

#include "EntityManager.h"
#include <cstdint>

class EntitySystem {
public:

    EntitySystem(EntityManager* manager, int32_t priority) : priority(priority), manager(manager) {}

    virtual void initialize() {}

    virtual void addEntity(uint32_t id) {}

    virtual void removeEntity(uint32_t id) {}

    virtual void refreshEntity(uint32_t id) {}

    virtual void process(float dt) {}

    virtual ~EntitySystem() = 0;

    const int32_t priority;

protected:
    EntityManager* manager;

};

#endif // ENTITYSYSTEM_H_INCLUDED
