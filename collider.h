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
    CollisionPoint center;
    float radius;
};

struct Collider {

    static const std::string name;

    Collider();

    enum class CollisionGroup {
        none,
        Player,
        PlayerBullet,
        Enemy,
        EnemyBullet

    };

    enum class ColliderType {
        none,
        AABB,
        Point,
        Circle
    };

    CollisionGroup collisionGroup;
    ColliderType colliderType;

    union {
        AABoundingBox aabb;
        CollisionPoint point;
        CollisionCircle circle;

    };


};


#endif // COLLIDER_H_INCLUDED
