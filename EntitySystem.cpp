#include "EntitySystem.h"

EntitySystem::EntitySystem(EntityManager* manager, int32_t priority) : priority(priority), manager(manager) {
    manager->addSystem(this);
}

EntitySystem::~EntitySystem() {}
