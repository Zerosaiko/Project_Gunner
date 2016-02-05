#include "collider.h"

const std::string Collider::name{"collider"};

Collider::Collider() : collisionGroup(Collider::CollisionGroup::none), colliderType(Collider::ColliderType::none) {}

template<>
Collider buildFromString<Collider>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    Collider c;
    if (str[pos] == "Player") {
        c.collisionGroup = Collider::CollisionGroup::Player;
    } else if (str[pos] == "PlayerBullet") {
        c.collisionGroup = Collider::CollisionGroup::PlayerBullet;
    } else if (str[pos] == "Enemy") {
        c.collisionGroup = Collider::CollisionGroup::Enemy;
    } else if (str[pos] == "EnemyBullet") {
        c.collisionGroup = Collider::CollisionGroup::EnemyBullet;
    }

    if (str[++pos] == "Point") {
        c.colliderType = Collider::ColliderType::Point;
        c.point.x = c.point.y = 0.f;
        c.point.x = buildFromString<float>(str, ++pos);
        c.point.y = buildFromString<float>(str, ++pos);
    } else if (str[pos] == "AABB") {
        c.colliderType = Collider::ColliderType::AABB;
        c.aabb.minX = c.aabb.minY = c.aabb.maxX = c.aabb.maxY = 0.f;
        c.aabb.minX = buildFromString<float>(str, ++pos);
        c.aabb.minY = buildFromString<float>(str, ++pos);
        c.aabb.maxX = buildFromString<float>(str, ++pos);
        c.aabb.maxY = buildFromString<float>(str, ++pos);
    } else if (str[pos] == "Circle") {
        c.colliderType = Collider::ColliderType::Circle;
        c.circle.center.x = c.circle.center.y = c.circle.radius = 0.f;
        c.circle.center.x = buildFromString<float>(str, ++pos);
        c.circle.center.y = buildFromString<float>(str, ++pos);
        c.circle.radius = buildFromString<float>(str, ++pos);
    }

    return c;
}
