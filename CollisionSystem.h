#ifndef COLLISIONSYSTEM_H_INCLUDED
#define COLLISIONSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include "collider.h"

class CollisionSystem : public EntitySystem {

public:
    CollisionSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);
private:

    std::unordered_map<uint32_t, std::vector<EntityManager::component_pair const *>::size_type> entityIDs;

    std::vector<std::vector<EntityManager::component_pair const *>::size_type> freeIDXs;

    std::vector<EntityManager::component_pair const *> entities;

    std::vector<Component<Collider::name, Collider>>* velocityPool;

};

#endif // COLLISIONSYSTEM_H_INCLUDED
