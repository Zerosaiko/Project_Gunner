#ifndef COLLIDER_H_INCLUDED
#define COLLIDER_H_INCLUDED

#include "component.h"

#include <unordered_map>


struct AABoundingBox {
    float minX, minY, maxX, maxY;

};

struct CollisionPoint {
    float x, y;
};

struct OBoundingBox {

    std::array<CollisionPoint, 4> vertices;

    float angle;

    CollisionPoint pivot;

    void setAngle(float angle);

    void rotate(float angle);

};

struct CollisionCircle {
    float radius;
};

struct Collider {

    static const std::string name;

    Collider();

    enum ColliderType : uint8_t {
        NoType = (uint8_t)-1,
        AABB = 0,
        Point = 1,
        Circle = 2,
        OBB = 3
    };

    std::string collisionGroup;
    ColliderType colliderType;
    CollisionPoint position;
    CollisionPoint offset;

    union {
        AABoundingBox aabb;
        CollisionCircle circle;
        OBoundingBox obb;
    };

    AABoundingBox spatialBox;
    sol::object collisionHandlers;

};


#endif // COLLIDER_H_INCLUDED
