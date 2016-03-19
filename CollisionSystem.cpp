#include "CollisionSystem.h"
#include "EntityManager.h"
#include "SDL.h"

CollisionSystem::CollisionSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    colliderPool = manager->getComponentPool<Component<Collider::name, Collider>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);

    collisionGroups.resize(5);

    collisionTable[Collider::ColliderType::AABB][Collider::ColliderType::AABB] = aabbToaabbCollision;
    collisionTable[Collider::ColliderType::AABB][Collider::ColliderType::Point] = aabbToPointCollision;
    collisionTable[Collider::ColliderType::AABB][Collider::ColliderType::Circle] = aabbToCircleCollision;

    collisionTable[Collider::ColliderType::Point][Collider::ColliderType::AABB] = pointToaabbCollision;
    collisionTable[Collider::ColliderType::Point][Collider::ColliderType::Point] = pointToPointCollision;
    collisionTable[Collider::ColliderType::Point][Collider::ColliderType::Circle] = pointToCircleCollision;

    collisionTable[Collider::ColliderType::Circle][Collider::ColliderType::AABB] = circleToaabbCollision;
    collisionTable[Collider::ColliderType::Circle][Collider::ColliderType::Point] = circleToPointCollision;
    collisionTable[Collider::ColliderType::Circle][Collider::ColliderType::Circle] = circleToCircleCollision;
}

void CollisionSystem::initialize() {}

void CollisionSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto collider = entity->find("collider");
        auto position = entity->find("position");
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ( (delay == entity->end() || !delay->second.active) && (pause == entity->end() || !pause->second.active)
            && collider != entity->end() && position!= entity->end()
            && collider->second.active && position->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&collider->second, &position->second);

            Collider& col = (*colliderPool)[collider->second.index].data;
            collisionGroups[col.collisionGroup].insert(id);

        }
    }
}

void CollisionSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
    for (auto& cGroup: collisionGroups) {
        cGroup.erase(id);
    }

}

void CollisionSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity.first->active && entity.second->active )) {
        removeEntity(id);
    } else {
        const auto& fullEntity = manager->getEntity(id);
        auto delay = fullEntity->find("fullDelay");
        auto pause = fullEntity->find("pauseDelay");
        if ( (delay != fullEntity->end() && delay->second.active) || (pause != fullEntity->end() && pause->second.active) ) {
            removeEntity(id);
        } else if (entity.first->dirty) {

            Collider& col = (*colliderPool)[entity.first->index].data;
            collisionGroups[col.collisionGroup].insert(id);
            for (size_t i = 0; i < collisionGroups.size(); ++i) {
                if (col.collisionGroup != i)
                    collisionGroups[i].erase(id);
            }

        }
    }

}

void CollisionSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    auto& playerColliders = collisionGroups[Collider::CollisionGroup::Player];
    auto& playerBulletColliders = collisionGroups[Collider::CollisionGroup::PlayerBullet];
    auto& enemyColliders = collisionGroups[Collider::CollisionGroup::Enemy];
    auto& enemyBulletColliders = collisionGroups[Collider::CollisionGroup::EnemyBullet];
    auto& pickupColliders = collisionGroups[Collider::CollisionGroup::Pickup];
    for (auto playerIT = playerColliders.begin(); playerIT != playerColliders.end(); ++playerIT) {
        uint32_t id = *playerIT;
        Collider& playerCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
        Position& playerPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
        playerCollider.position.x = playerPosition.posX;
        playerCollider.position.y = playerPosition.posY;
        for (auto enemyIT = enemyColliders.begin(); enemyIT != enemyColliders.end(); ++enemyIT) {
            uint32_t id = *enemyIT;
            Collider& enemyCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
            Position& enemyPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
            enemyCollider.position.x = enemyPosition.posX;
            enemyCollider.position.y = enemyPosition.posY;
            if (collisionTable[playerCollider.colliderType][enemyCollider.colliderType](playerCollider, enemyCollider)) {
                // probably do something to kill the player unless there's some kind of shield
            }
        }
        for (auto enemyBulletIT = enemyBulletColliders.begin(); enemyBulletIT != enemyBulletColliders.end(); ++enemyBulletIT) {
            uint32_t id = *enemyBulletIT;
            Collider& enemyBulletCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
            Position& enemyBulletPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
            enemyBulletCollider.position.x = enemyBulletPosition.posX;
            enemyBulletCollider.position.y = enemyBulletPosition.posY;
            if (collisionTable[playerCollider.colliderType][enemyBulletCollider.colliderType](playerCollider, enemyBulletCollider)) {
                // probably do something to kill the player unless there's some kind of shield
            }
        }
        for (auto pickupIT = pickupColliders.begin(); pickupIT != pickupColliders.end(); ++pickupIT) {
            uint32_t id = *pickupIT;
            Collider& pickupCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
            Position& pickupPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
            pickupCollider.position.x = pickupPosition.posX;
            pickupCollider.position.y = pickupPosition.posY;
            if (collisionTable[playerCollider.colliderType][pickupCollider.colliderType](playerCollider, pickupCollider)) {
                // do whatever the pickup does
            }
        }
    }
    for (auto playerBulletIT = playerBulletColliders.begin(); playerBulletIT != playerBulletColliders.end(); ++playerBulletIT) {
        uint32_t id = *playerBulletIT;
        Collider& playerBulletCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
        Position& playerBulletPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
        playerBulletCollider.position.x = playerBulletPosition.posX;
        playerBulletCollider.position.y = playerBulletPosition.posY;
        for (auto enemyIT = enemyColliders.begin(); enemyIT != enemyColliders.end(); ++enemyIT) {
            uint32_t id = *enemyIT;
            Collider& enemyCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
            Position& enemyPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
            enemyCollider.position.x = enemyPosition.posX;
            enemyCollider.position.y = enemyPosition.posY;
            if (collisionTable[playerBulletCollider.colliderType][enemyCollider.colliderType](playerBulletCollider, enemyCollider)) {
                // probably do something to lower the health of the enemy
            }
        }
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "C-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';
}

float dst2(float x1, float y1, float x2, float y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

bool pointToPointCollision(Collider& p1, Collider& p2) {
    float xDiff = p1.position.x + p1.offset.x - p2.position.x - p2.offset.x;
    float yDiff = p1.position.y + p1.offset.y - p2.position.y - p2.offset.y;
    return xDiff <= 0.001f && xDiff >= -0.001f && yDiff <= 0.001f && yDiff >= -0.001f;
}

bool pointToaabbCollision(Collider& p, Collider& r) {
    float posX = p.position.x + p.offset.x;
    float posY = p.position.y + p.offset.y;
    float minX = r.position.x + r.aabb.minX + r.offset.x;
    float maxX = r.position.x + r.aabb.maxX + r.offset.x;
    float minY = r.position.y + r.aabb.minY + r.offset.y;
    float maxY = r.position.y + r.aabb.maxY + r.offset.y;
    return (posX >= minX && posX <= maxX
        && posY >= minY && posY <= maxY)
        || ( ( (minX >= posX && minX <= posX + 1) || (maxX >= posX && maxX <= posX + 1) )
        && ( (minY >= posY && minY <= posY + 1) || (maxY >= posY && maxY <= posY + 1) ) );
}

bool pointToCircleCollision(Collider& p, Collider& c) {
    float posX = p.position.x + p.offset.x;
    float posY = p.position.y + p.offset.y;
    float centerX = c.position.x + c.offset.x;
    float centerY = c.position.y + c.offset.y;
    return dst2(posX, posY, centerX, centerY) <= (c.circle.radius * c.circle.radius);
}

bool aabbToaabbCollision(Collider& r1, Collider& r2) {
    float minX1 = r1.position.x + r1.aabb.minX + r1.offset.x;
    float maxX1 = r1.position.x + r1.aabb.maxX + r1.offset.x;
    float minY1 = r1.position.y + r1.aabb.minY + r1.offset.y;
    float maxY1 = r1.position.y + r1.aabb.maxY + r1.offset.y;
    float minX2 = r2.position.x + r2.aabb.minX + r2.offset.x;
    float maxX2 = r2.position.x + r2.aabb.maxX + r2.offset.x;
    float minY2 = r2.position.y + r2.aabb.minY + r2.offset.y;
    float maxY2 = r2.position.y + r2.aabb.maxY + r2.offset.y;
    return
            (
                ( (minX1 >= minX2 && minX1 <= maxX2)
                    || (maxX1 >= minX2 && maxX1 <= maxX2) )
                && ( (minY1 >= minY2 && minY1 <= maxY2)
                    || (maxY1 >= minY2 && maxY1 <= maxY2) ) )

            ||
            (
                ( (minX2 >= minX1 && minX2 <= maxX1)
                    || (maxX2 >= minX1 && maxX2 <= maxX1) )
                && ( (minY2 >= minY1 && minY2 <= maxY1)
                    || (maxY2 >= minY1 && maxY2 <= maxY1) ) );
}

bool aabbToPointCollision(Collider& r1, Collider& p2) {
    return pointToaabbCollision(p2, r1);
}

bool aabbToCircleCollision(Collider& r, Collider& c) {
    float minX = r.position.x + r.aabb.minX + r.offset.x;
    float maxX = r.position.x + r.aabb.maxX + r.offset.x;
    float minY = r.position.y + r.aabb.minY + r.offset.y;
    float maxY = r.position.y + r.aabb.maxY + r.offset.y;
    float rad2 = c.circle.radius * c.circle.radius;
    float centerX = c.position.x + c.offset.x;
    float centerY = c.position.y + c.offset.y;
    return dst2(minX, minY, centerX, centerY) <= rad2
        || dst2(maxX, minY, centerX, centerY) <= rad2
        || dst2(minX, maxY, centerX, centerY) <= rad2
        || dst2(maxX, maxY, centerX, centerY) <= rad2;
}

bool circleToCircleCollision(Collider& c1, Collider& c2) {
    float centerX1 = c1.position.x + c1.offset.x;
    float centerY1 = c1.position.y + c1.offset.y;
    float centerX2 = c2.position.x + c2.offset.x;
    float centerY2 = c2.position.y + c2.offset.y;
    return dst2(centerX1, centerY1, centerX2, centerY2) <= (c1.circle.radius * c1.circle.radius);
}

bool circleToPointCollision(Collider& c, Collider& p) {
    return pointToCircleCollision(p, c);
}

bool circleToaabbCollision(Collider& c, Collider& r) {
    return aabbToCircleCollision(r, c);
}
