#ifndef LIFETIMERSYSTEM_H_INCLUDED
#define LIFETIMERSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include "lifeTimer.h"
#include <vector>
#include <deque>

class LifeTimerSystem : public EntitySystem {

public:
    LifeTimerSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<EntityManager::ComponentHandle const *> entities;

    std::weak_ptr<std::deque<Component<lifeTimerName, float>>> lifeTimerPool;

};

#endif // LIFETIMERSYSTEM_H_INCLUDED
