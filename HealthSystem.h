#ifndef HEALTHSYSTEM_H_INCLUDED
#define HEALTHSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include "sol.hpp"
#include "health.h"
class HealthSystem : public EntitySystem {

public:
    HealthSystem(EntityManager* const manager, int32_t priority, sol::state &luaState);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

    bool changeHealth(uint32_t id, int16_t amt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<EntityManager::ComponentHandle const *> entities;

    std::weak_ptr<std::deque<Component<Health::name, Health>>> healthPool;
};

#endif // HEALTHSYSTEM_H_INCLUDED
