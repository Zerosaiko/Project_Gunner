#ifndef COLLIDER_H_INCLUDED
#define COLLIDER_H_INCLUDED

#include "component.h"

struct AABoundingBox {
    float minX, minY, maxX, maxY;

};

struct CollisionPoint {
    float x, y;
};

struct CollisionCircle {
    float radius;
};

struct Collider {

    static const std::string name;

    Collider();

    enum CollisionGroup : size_t {
        NoGroup = (size_t)-1,
        Player = 0,
        PlayerBullet = 1,
        Enemy = 2,
        EnemyBullet = 3,
        Pickup = 4,
        GroupSize

    };

    enum ColliderType : size_t {
        NoType = (size_t)-1,
        AABB = 0,
        Point = 1,
        Circle = 2
    };

    CollisionGroup collisionGroup;
    ColliderType colliderType;
    CollisionPoint position;
    CollisionPoint offset;

    union {
        AABoundingBox aabb;
        CollisionCircle circle;

    };


};


#endif // COLLIDER_H_INCLUDED
