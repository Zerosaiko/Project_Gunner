#ifndef MOVEMENTSYSTEM_H_INCLUDED
#define MOVEMENTSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include <unordered_map>
#include "displace.h"

class MovementSystem : public EntitySystem {

public:
    MovementSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::unordered_map<uint32_t, std::vector<EntityManager::component_pair const *>::size_type> entityIDs;

    std::vector<std::vector<EntityManager::component_pair const *>::size_type> freeIDXs;

    std::vector<EntityManager::component_pair const *> entities;

    std::vector<Displace>* displacePool;

};

#endif // MOVEMENTSYSTEM_H_INCLUDED
