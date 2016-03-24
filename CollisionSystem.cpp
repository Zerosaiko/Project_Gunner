#include "CollisionSystem.h"
#include "EntityManager.h"
#include "SDL.h"

CollisionSystem::CollisionSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority}, collisionWidth(360), collisionHeight(480),
        gridWidth(collisionWidth / spatialCollisionGroups.size()), gridHeight(collisionHeight / spatialCollisionGroups[0].size()) {
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    colliderPool = manager->getComponentPool<Component<Collider::name, Collider>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);

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
            Position& pos = positionPool->operator[](position->second.index).data;

            int32_t minX = int32_t(col.spatialBox.minX + pos.posX), maxX = int32_t(col.spatialBox.maxX + pos.posX),
            minY = int32_t(col.spatialBox.minY + pos.posY), maxY = int32_t(col.spatialBox.maxY + pos.posY);

            if (minX < 0) minX = 0; if (minX > collisionWidth - 1) minX = collisionWidth - 1;
            if (maxX < 0) maxX = 0; if (maxX > collisionWidth - 1) maxX = collisionWidth - 1;
            if (minY < 0) minY = 0; if (minY > collisionHeight - 1) minY = collisionHeight - 1;
            if (maxY < 0) maxY = 0; if (maxY > collisionHeight - 1) maxY = collisionHeight - 1;

            size_t spatialMinX = minX / gridWidth, spatialMaxX = maxX / gridWidth, spatialMinY = minY / gridHeight, spatialMaxY = maxY / gridHeight;

            spatialIndices.emplace_back(spatialMinX, spatialMaxX, spatialMinY, spatialMaxY);

            if (spatialMinX == spatialMaxX && spatialMinY == spatialMaxY) {
                spatialCollisionGroups[spatialMinX][spatialMinY][col.collisionGroup].insert(id);
            } else {
                collisionGroups[col.collisionGroup].insert(id);
            }
        }
    }
}

void CollisionSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    for (auto& cGroup: collisionGroups) {
        cGroup.erase(id);
    }

    SpatialIndices& oldIndices = spatialIndices[entityIDXs[id]];

    if (oldIndices.minX == oldIndices.maxX && oldIndices.minY == oldIndices.maxY) {
        for (auto& cGroup: spatialCollisionGroups[oldIndices.minX][oldIndices.minY]) {
            cGroup.erase(id);
        }
    }

    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    spatialIndices[entityIDXs[id]] = spatialIndices.back();
    spatialIndices.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;

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
        } else if (entity.first->dirty || entity.second->dirty) {

            Collider& col = (*colliderPool)[entity.first->index].data;
            Position& pos = positionPool->operator[](entity.second->index).data;

            if (entity.first->dirty || entity.second->dirty) {

                int32_t minX = int32_t(col.spatialBox.minX + pos.posX), maxX = int32_t(col.spatialBox.maxX + pos.posX),
                minY = int32_t(col.spatialBox.minY + pos.posY), maxY = int32_t(col.spatialBox.maxY + pos.posY);

                if (minX < 0) minX = 0; if (minX > collisionWidth - 1) minX = collisionWidth - 1;
                if (maxX < 0) maxX = 0; if (maxX > collisionWidth - 1) maxX = collisionWidth - 1;
                if (minY < 0) minY = 0; if (minY > collisionHeight - 1) minY = collisionHeight - 1;
                if (maxY < 0) maxY = 0; if (maxY > collisionHeight - 1) maxY = collisionHeight - 1;

                size_t spatialMinX = minX / gridWidth, spatialMaxX = maxX / gridWidth, spatialMinY = minY / gridHeight, spatialMaxY = maxY / gridHeight;

                SpatialIndices& oldIndices = spatialIndices[entityIDXs[id]];

                if (spatialMinX != oldIndices.minX || spatialMaxX != oldIndices.maxX || spatialMinY != oldIndices.minY
                    || spatialMaxY != oldIndices.maxY) {

                    for (size_t i = 0; i < collisionGroups.size(); ++i) {
                        if (col.collisionGroup != i) {
                            collisionGroups[i].erase(id);
                            for (size_t j = 0; j < spatialCollisionGroups.size(); ++j) {
                                for (size_t k = 0; k < spatialCollisionGroups[i].size(); ++k) {
                                    spatialCollisionGroups[j][k][i].erase(id);
                                }
                            }
                        }
                    }

                    if (spatialMinX == spatialMaxX && spatialMinY == spatialMaxY) {
                        spatialCollisionGroups[spatialMinX][spatialMinY][col.collisionGroup].insert(id);
                    } else {
                        collisionGroups[col.collisionGroup].insert(id);
                    }
                    oldIndices.minX = spatialMinX; oldIndices.maxX = spatialMaxX;
                    oldIndices.minY = spatialMinY; oldIndices.maxY = spatialMaxY;
                }

            }

        }
    }

}

void CollisionSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    for (size_t idx = 0; idx < entities.size(); ++idx) {

        const auto& entity = entities[idx];
        const Position& position = (*positionPool)[entity.second->index].data;
        Collider& collider = (*colliderPool)[entity.first->index].data;

        int32_t minX = int32_t(collider.spatialBox.minX + position.posX), maxX = int32_t(collider.spatialBox.maxX + position.posX),
        minY = int32_t(collider.spatialBox.minY + position.posY), maxY = int32_t(collider.spatialBox.maxY + position.posY);

        if (minX < 0) minX = 0; if (minX > collisionWidth - 1) minX = collisionWidth - 1;
        if (maxX < 0) maxX = 0; if (maxX > collisionWidth - 1) maxX = collisionWidth - 1;
        if (minY < 0) minY = 0; if (minY > collisionHeight - 1) minY = collisionHeight - 1;
        if (maxY < 0) maxY = 0; if (maxY > collisionHeight - 1) maxY = collisionHeight - 1;

        size_t spatialMinX = minX / gridWidth, spatialMaxX = maxX / gridWidth, spatialMinY = minY / gridHeight, spatialMaxY = maxY / gridHeight;

        SpatialIndices& oldIndices = spatialIndices[idx];

        if (spatialMinX != oldIndices.minX || spatialMaxX != oldIndices.maxX || spatialMinY != oldIndices.minY
            || spatialMaxY != oldIndices.maxY) {

            collisionGroups[collider.collisionGroup].erase(idxToID[idx]);

            spatialCollisionGroups[oldIndices.maxX][oldIndices.maxY][collider.collisionGroup].erase(idxToID[idx]);

            if (spatialMinX == spatialMaxX && spatialMinY == spatialMaxY) {
                spatialCollisionGroups[spatialMinX][spatialMinY][collider.collisionGroup].insert(idxToID[idx]);
            } else {
                collisionGroups[collider.collisionGroup].insert(idxToID[idx]);
            }
            oldIndices.minX = spatialMinX; oldIndices.maxX = spatialMaxX;
            oldIndices.minY = spatialMinY; oldIndices.maxY = spatialMaxY;
        }

    }

    checkPlayerCollisions();

    checkPlayerBulletCollisions();

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "C-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';
}

void CollisionSystem::checkPlayerCollisions() {

    auto& playerColliders = collisionGroups[Collider::CollisionGroup::Player];
    checkPlayerCollisions(playerColliders);

    for(size_t i = 0; i < spatialCollisionGroups.size(); ++i) {
        for (size_t j = 0; j < spatialCollisionGroups[i].size(); ++j) {
            checkPlayerCollisions(spatialCollisionGroups[i][j][Collider::CollisionGroup::Player]);
        }
    }

}

void CollisionSystem::checkPlayerCollisions(std::unordered_set<uint32_t>& playerColliders) {

    auto& enemyColliders = collisionGroups[Collider::CollisionGroup::Enemy];
    auto& enemyBulletColliders = collisionGroups[Collider::CollisionGroup::EnemyBullet];
    auto& pickupColliders = collisionGroups[Collider::CollisionGroup::Pickup];

    for (auto playerIT = playerColliders.begin(); playerIT != playerColliders.end(); ++playerIT) {
        uint32_t id = *playerIT;
        Collider& playerCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
        Position& playerPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
        playerCollider.position.x = playerPosition.posX;
        playerCollider.position.y = playerPosition.posY;

        auto& spatialIndex = spatialIndices[idxToID[id]];

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

        for (size_t i = spatialIndex.minX; i <= spatialIndex.maxX; ++i) {
            for (size_t j = spatialIndex.minY; j <= spatialIndex.maxY; ++j) {
                auto& enemyColliders = spatialCollisionGroups[i][j][Collider::CollisionGroup::Enemy];
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

        for (size_t i = spatialIndex.minX; i <= spatialIndex.maxX; ++i) {
            for (size_t j = spatialIndex.minY; j <= spatialIndex.maxY; ++j) {
                auto& enemyBulletColliders = spatialCollisionGroups[i][j][Collider::CollisionGroup::EnemyBullet];
                for (auto enemyIT = enemyBulletColliders.begin(); enemyIT != enemyBulletColliders.end(); ++enemyIT) {
                    uint32_t id = *enemyIT;
                    Collider& enemyBulletCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
                    Position& enemyBulletPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
                    enemyBulletCollider.position.x = enemyBulletPosition.posX;
                    enemyBulletCollider.position.y = enemyBulletPosition.posY;
                    if (collisionTable[playerCollider.colliderType][enemyBulletCollider.colliderType](playerCollider, enemyBulletCollider)) {
                        // probably do something to kill the player unless there's some kind of shield
                    }
                }
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

        for (size_t i = spatialIndex.minX; i <= spatialIndex.maxX; ++i) {
            for (size_t j = spatialIndex.minY; j <= spatialIndex.maxY; ++j) {
                auto& pickupColliders = spatialCollisionGroups[i][j][Collider::CollisionGroup::Pickup];
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
        }
    }
}

void CollisionSystem::checkPlayerBulletCollisions() {
    auto& playerBulletColliders = collisionGroups[Collider::CollisionGroup::PlayerBullet];
    checkPlayerBulletCollisions(playerBulletColliders);

    for(size_t i = 0; i < spatialCollisionGroups.size(); ++i) {
        for (size_t j = 0; j < spatialCollisionGroups[i].size(); ++j) {
            checkPlayerCollisions(spatialCollisionGroups[i][j][Collider::CollisionGroup::PlayerBullet]);
        }
    }

}

void CollisionSystem::checkPlayerBulletCollisions(std::unordered_set<uint32_t>& playerBulletColliders) {


    auto& enemyColliders = collisionGroups[Collider::CollisionGroup::Enemy];

    for (auto playerBulletIT = playerBulletColliders.begin(); playerBulletIT != playerBulletColliders.end(); ++playerBulletIT) {
        uint32_t id = *playerBulletIT;
        Collider& playerBulletCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
        Position& playerBulletPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
        playerBulletCollider.position.x = playerBulletPosition.posX;
        playerBulletCollider.position.y = playerBulletPosition.posY;

        auto& spatialIndex = spatialIndices[idxToID[id]];

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

        for (size_t i = spatialIndex.minX; i <= spatialIndex.maxX; ++i) {
            for (size_t j = spatialIndex.minY; j <= spatialIndex.maxY; ++j) {
                auto& enemyColliders = spatialCollisionGroups[i][j][Collider::CollisionGroup::Enemy];
                for (auto enemyIT = enemyColliders.begin(); enemyIT != enemyColliders.end(); ++enemyIT) {
                    uint32_t id = *enemyIT;
                    Collider& enemyCollider = (*colliderPool)[entities[entityIDXs[id]].first->index].data;
                    Position& enemyPosition = (*positionPool)[entities[entityIDXs[id]].second->index].data;
                    enemyCollider.position.x = enemyPosition.posX;
                    enemyCollider.position.y = enemyPosition.posY;
                    if (collisionTable[playerBulletCollider.colliderType][enemyCollider.colliderType](playerBulletCollider, enemyCollider)) {
                        // probably do something to kill the player unless there's some kind of shield
                    }
                }
            }
        }
    }
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
    float aabbWidthGrowth = (r2.aabb.maxX - r2.aabb.minX) / 2;
    float aabbHeightGrowth = (r2.aabb.maxY - r2.aabb.minY) / 2;
    float minX = r1.position.x + r1.aabb.minX + r1.offset.x - aabbWidthGrowth;
    float maxX = r1.position.x + r1.aabb.maxX + r1.offset.x + aabbWidthGrowth;
    float minY = r1.position.y + r1.aabb.minY + r1.offset.y - aabbHeightGrowth;
    float maxY = r1.position.y + r1.aabb.maxY + r1.offset.y + aabbHeightGrowth;
    float posX = r2.position.x + r2.offset.x;
    float posY = r2.position.y + r2.offset.y;
    return (posX >= minX && posX <= maxX
        && posY >= minY && posY <= maxY)
        || ( ( (minX >= posX && minX <= posX + 1) || (maxX >= posX && maxX <= posX + 1) )
        && ( (minY >= posY && minY <= posY + 1) || (maxY >= posY && maxY <= posY + 1) ) );
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
