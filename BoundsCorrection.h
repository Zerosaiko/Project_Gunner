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

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<std::pair<EntityManager::ComponentHandle const *,EntityManager::ComponentHandle const *>> entities;

    std::deque<Component<Position::name, Position>>* positionPool;

    std::deque<Component<Bounds::name, Bounds>>* boundsPool;

};

#endif // BOUNDSCORRECTION_H_INCLUDED
