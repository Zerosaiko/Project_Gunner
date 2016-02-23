#ifndef DELAYSYSTEM_H_INCLUDED
#define DELAYSYSTEM_H_INCLUDED

#include "delayComponent.h"
#include "EntitySystem.h"

class DelaySystem : public EntitySystem {

public:
    DelaySystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<EntityManager::component_pair const *> entities;

    std::vector<Component<delayComponent::fullDelay, float>>* delayPool;

};

#endif // DELAYSYSTEM_H_INCLUDED
