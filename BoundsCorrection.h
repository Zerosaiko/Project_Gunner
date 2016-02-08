#ifndef BOUNDSCORRECTION_H_INCLUDED
#define BOUNDSCORRECTION_H_INCLUDED

#include "EntitySystem.h"
#include <unordered_map>
#include "displace.h"
#include "boundsComponent.h"

class BoundsSystem : public EntitySystem {

public:
    BoundsSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::unordered_map<uint32_t, size_t> entityIDs;

    std::vector<std::vector<std::pair<EntityManager::component_pair const *,EntityManager::component_pair const *>>::size_type> freeIDXs;

    std::vector<std::pair<EntityManager::component_pair const *,EntityManager::component_pair const *>> entities;

    std::vector<Component<Position::name, Position>>* positionPool;

    std::vector<Component<Bounds::name, Bounds>>* boundsPool;

};

#endif // BOUNDSCORRECTION_H_INCLUDED
