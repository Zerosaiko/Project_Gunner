#ifndef MOVEMENTINPUT_H_INCLUDED
#define MOVEMENTINPUT_H_INCLUDED

#include "EntitySystem.h"
#include "displace.h"
#include "playerComponents.h"
#include "InputMap.h"

class MovementInputSystem : public EntitySystem {

public:
    MovementInputSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::unordered_map<uint32_t, std::vector<std::pair<EntityManager::ComponentHandle const *, EntityManager::ComponentHandle const *>>::size_type> entityIDs;

    std::vector<std::vector<std::pair<EntityManager::ComponentHandle const *, EntityManager::ComponentHandle const *>>::size_type> freeIDXs;

    std::vector<std::pair<EntityManager::ComponentHandle const *, EntityManager::ComponentHandle const *>> entities;

    std::deque<Component<Velocity::name, Velocity>>* velocityPool;

    std::deque<Component<PlayerCmp::name, PlayerCmp>>* playerPool;

};

#endif // MOVEMENTINPUT_H_INCLUDED
