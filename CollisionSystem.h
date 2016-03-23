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

    struct SpatialIndices {
        size_t minX, maxX, minY, maxY;

        SpatialIndices(size_t minX, size_t maxX, size_t minY, size_t maxY) : minX(minX), maxX(maxX), minY(minY), maxY(maxY) {}

    };

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<std::pair<EntityManager::ComponentHandle const *,EntityManager::ComponentHandle const *>> entities;

    std::vector<SpatialIndices> spatialIndices;

    std::deque<Component<Position::name, Position>>* positionPool;

    std::deque<Component<Collider::name, Collider>>* colliderPool;

    std::function<bool(Collider&, Collider&)> collisionTable[3][3];

    std::array<std::unordered_set<uint32_t>, Collider::CollisionGroup::GroupSize> collisionGroups;

    std::array < std::array< std::array< std::unordered_set<uint32_t>, Collider::CollisionGroup::GroupSize >, 5 >, 5 > spatialCollisionGroups;

    int32_t collisionWidth;

    int32_t collisionHeight;

    int32_t gridWidth;

    int32_t gridHeight;

    void checkPlayerCollisions();

    void checkPlayerCollisions(std::unordered_set<uint32_t>& playerColliders);

    void checkPlayerBulletCollisions();

    void checkPlayerBulletCollisions(std::unordered_set<uint32_t>& playerColliders);

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
