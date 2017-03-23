#ifndef COLLISIONSYSTEM_H_INCLUDED
#define COLLISIONSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include "collider.h"
#include "displace.h"
#include "Transform.h"
#include "sol.hpp"
#include <functional>

class CollisionSystem : public EntitySystem {

public:
    CollisionSystem(EntityManager* const manager, int32_t priority, sol::state &luaState);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);
private:

    typedef std::unordered_set<uint32_t> CollisionSet;

    typedef std::vector<CollisionSet> CollisionGroup;

    typedef std::unordered_map<uint8_t, std::vector<uint8_t>> CollisionGraph;

    using CollisionFunc = bool(*)(Collider const&, Collider const&);

    struct SpatialIndices {
        size_t minX, maxX, minY, maxY;

        SpatialIndices(size_t minX, size_t maxX, size_t minY, size_t maxY) : minX(minX), maxX(maxX), minY(minY), maxY(maxY) {}

    };

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<std::pair<EntityManager::ComponentHandle const *,EntityManager::ComponentHandle const *>> entities;

    std::vector<SpatialIndices> spatialIndices;

    std::weak_ptr<std::deque<Component<Transform::name, Transform>>> positionPool;

    std::weak_ptr<std::deque<Component<Collider::name, Collider>>> colliderPool;

    sol::state &luaState;

    CollisionFunc collisionTable[3][3];

    std::unordered_map<std::string, uint8_t> groupIDs;

    std::unordered_map<uint8_t, std::string> groupIDToName;

    CollisionGraph collisionGraph;

    CollisionGroup collisionGroups;

    std::vector < std::vector< CollisionGroup >> spatialCollisionGroups;

    int32_t collisionWidth;

    int32_t collisionHeight;

    int32_t gridWidth;

    int32_t gridHeight;

    void checkCollisions(uint8_t group1ID, std::vector<uint8_t> &group2IDs);

    void checkCollisions(CollisionSet &group1, uint8_t group2ID);

    void handleCollision(std::string group1Name, std::string group2Name, uint32_t id1, uint32_t id2, Collider &col1, Collider &col2);

};

float dst2(const float x1, const float y1, const float x2, const float y2);

bool pointToPointCollision(Collider const &p1, Collider const &p2);

bool pointToaabbCollision(Collider const &p, Collider const &r);

bool pointToCircleCollision(Collider const &p, Collider const &c);

bool aabbToaabbCollision(Collider const &r1, Collider const &r2);

bool aabbToPointCollision(Collider const &r, Collider const &p);

bool aabbToCircleCollision(Collider const &r, Collider const &c);

bool circleToCircleCollision(Collider const &c1, Collider const &c2);

bool circleToPointCollision(Collider const &c, Collider const &p);

bool circleToaabbCollision(Collider const &c, Collider const &r);

#endif // COLLISIONSYSTEM_H_INCLUDED
