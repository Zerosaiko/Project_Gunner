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

    std::unordered_map<uint32_t, std::vector<EntityManager::component_pair const *>::size_type> entityIDs;

    std::vector<std::vector<EntityManager::component_pair const *>::size_type> freeIDXs;

    std::vector<EntityManager::component_pair const *> entities;

    std::vector<Component<delayComponent::fullDelay, float>>* delayPool;

};

#endif // DELAYSYSTEM_H_INCLUDED
