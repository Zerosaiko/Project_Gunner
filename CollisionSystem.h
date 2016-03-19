#ifndef COLLISIONSYSTEM_H_INCLUDED
#define COLLISIONSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include "collider.h"
#include "displace.h"
#include <functional>

class CollisionSystem : public EntitySystem {

public:
    CollisionSystem(EntityManager* const manager, int32_t priority);

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

    std::deque<Component<Collider::name, Collider>>* colliderPool;

    std::function<bool(Collider&, Collider&)> collisionTable[3][3];

    std::vector<std::unordered_set<uint32_t>> collisionGroups;

};

float dst2(float x1, float y1, float x2, float y2);

bool pointToPointCollision(Collider& p1, Collider& p2);

bool pointToaabbCollision(Collider& p, Collider& r);

bool pointToCircleCollision(Collider& p, Collider& c);

bool aabbToaabbCollision(Collider& r1, Collider& r2);

bool aabbToPointCollision(Collider& r, Collider& p);

bool aabbToCircleCollision(Collider& r, Collider& c);

bool circleToCircleCollision(Collider& c1, Collider& c2);

bool circleToPointCollision(Collider& c, Collider& p);

bool circleToaabbCollision(Collider& c, Collider& r);

#endif // COLLISIONSYSTEM_H_INCLUDED
