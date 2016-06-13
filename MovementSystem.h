#ifndef MOVEMENTSYSTEM_H_INCLUDED
#define MOVEMENTSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include <unordered_set>
#include "displace.h"
#include "Transform.h"

//  Simply adds the velocity of the Displace object to the position, and then saves the old position for interpolation
class MovementSystem : public EntitySystem {

public:
    MovementSystem(EntityManager* const manager, int32_t priority, TransformTree& tfGraph);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint32_t> backIDs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<std::pair<EntityManager::ComponentHandle const *,EntityManager::ComponentHandle const *>> entities;

    std::deque<Component<Transform::name, Transform>>* tfPool;

    std::deque<Component<Velocity::name, Velocity>>* velocityPool;

    TransformTree& tfGraph;

};

#endif // MOVEMENTSYSTEM_H_INCLUDED
