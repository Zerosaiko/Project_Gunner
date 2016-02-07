#include "collider.h"

const std::string Collider::name{"collider"};

Collider::Collider() : collisionGroup(Collider::CollisionGroup::NoGroup), colliderType(Collider::ColliderType::NoType) {}

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
    } else if (str[pos] == "Pickup") {
        c.collisionGroup = Collider::CollisionGroup::Pickup;
    }

    c.position.x = c.position.y = 0.f;
    c.offset.x = c.offset.y = 0.f;
    if (str[++pos] == "Point") {
        c.colliderType = Collider::ColliderType::Point;
        c.offset.x = buildFromString<float>(str, ++pos);
        c.offset.y = buildFromString<float>(str, ++pos);
    } else if (str[pos] == "AABB") {
        c.colliderType = Collider::ColliderType::AABB;
        c.position.x = c.position.y = c.aabb.minX = c.aabb.minY = c.aabb.maxX = c.aabb.maxY = 0.f;
        c.offset.x = buildFromString<float>(str, ++pos);
        c.offset.y = buildFromString<float>(str, ++pos);
        c.aabb.minX = buildFromString<float>(str, ++pos);
        c.aabb.minY = buildFromString<float>(str, ++pos);
        c.aabb.maxX = buildFromString<float>(str, ++pos);
        c.aabb.maxY = buildFromString<float>(str, ++pos);
    } else if (str[pos] == "Circle") {
        c.colliderType = Collider::ColliderType::Circle;
        c.circle.radius = 0.0f;
        c.offset.x = buildFromString<float>(str, ++pos);
        c.offset.y = buildFromString<float>(str, ++pos);
        c.circle.radius = buildFromString<float>(str, ++pos);
    }

    return c;
}
