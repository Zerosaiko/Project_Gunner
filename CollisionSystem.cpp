#include "CollisionSystem.h"
#include "EntityManager.h"
#include "SDL.h"
#include "Message.h"

CollisionSystem::CollisionSystem(EntityManager* const manager, int32_t priority, sol::state &luaState) : EntitySystem{manager, priority}, luaState(luaState),
        spatialCollisionGroups(6, std::vector<CollisionGroup>(6, CollisionGroup())),
        collisionWidth(360), collisionHeight(480), gridWidth(collisionWidth / spatialCollisionGroups.size()),
        gridHeight(collisionHeight / spatialCollisionGroups[0].size()) {
    positionPool = manager->getComponentPool<Component<Transform::name, Transform>>();
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

    luaState["CollisionSys"] = luaState.create_table();
    luaState.script(R"(
        CollisionSys.CollisionGroups = {"Player", "PlayerBullet", "Enemy", "EnemyBullet", "Pickup"}
        CollisionSys.CollisionMatrices =
        {
            ["Player"] = {"Enemy", "EnemyBullet", "Pickup"},
            ["PlayerBullet"] = {"Enemy"}
        }
    )");
    sol::table tbl = luaState["CollisionSys"]["CollisionGroups"];
    collisionGroups.resize(tbl.size());

    for (auto &area : spatialCollisionGroups) {
        for (auto &group : area) {
            group.resize(tbl.size());
        }
    }
    int i = 0;
    for (auto &s : tbl) {
        groupIDs.emplace(s.second.as<std::string>(), i);
        groupIDToName.emplace(i++, s.second.as<std::string>());
    }
    sol::table colGraph = luaState["CollisionSys"]["CollisionMatrices"];
    for (auto &edges : colGraph) {
        std::vector<uint8_t> colliEdges;
        for (auto &edge : edges.second.as<sol::table>()) {
            colliEdges.emplace_back(groupIDs[edge.second.as<std::string>()]);
        }
        collisionGraph.emplace(groupIDs[edges.first.as<std::string>()], std::move(colliEdges));
    }
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
        const auto& components = entity->components;
        auto collider = components.find("collider");
        auto position = components.find("transform");
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay == components.end() || !delay->second.active) && (pause == components.end() || !pause->second.active)
            && collider != components.end() && position!= components.end()
            && collider->second.active && position->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&collider->second, &position->second);

            Collider& col = (*colliderPool.lock())[collider->second.index].data;
            Transform& pos = positionPool.lock()->operator[](position->second.index).data;
            float posX = 0, posY = 0;
            std::tie(posX, posY) = pos.worldPresent.getPos();

            int32_t minX = int32_t(col.spatialBox.minX + posX), maxX = int32_t(col.spatialBox.maxX + posX),
            minY = int32_t(col.spatialBox.minY + posY), maxY = int32_t(col.spatialBox.maxY + posY);

            if (minX < 0) minX = 0; if (minX > collisionWidth - 1) minX = collisionWidth - 1;
            if (maxX < 0) maxX = 0; if (maxX > collisionWidth - 1) maxX = collisionWidth - 1;
            if (minY < 0) minY = 0; if (minY > collisionHeight - 1) minY = collisionHeight - 1;
            if (maxY < 0) maxY = 0; if (maxY > collisionHeight - 1) maxY = collisionHeight - 1;

            size_t spatialMinX = minX / gridWidth, spatialMaxX = maxX / gridWidth, spatialMinY = minY / gridHeight, spatialMaxY = maxY / gridHeight;

            spatialIndices.emplace_back(spatialMinX, spatialMaxX, spatialMinY, spatialMaxY);

            auto collisionGroup = groupIDs[col.collisionGroup];
            if (spatialMinX == spatialMaxX && spatialMinY == spatialMaxY) {
                spatialCollisionGroups[spatialMinX][spatialMinY][collisionGroup].insert(id);
            } else {
                collisionGroups[collisionGroup].insert(id);
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
        auto fullEntity = manager->getEntity(id);
        const auto& components = fullEntity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay != components.end() && delay->second.active) || (pause != components.end() && pause->second.active) ) {
            removeEntity(id);
        } else if (entity.first->dirty || entity.second->dirty) {

            Collider& col = (*colliderPool.lock())[entity.first->index].data;
            Transform& pos = positionPool.lock()->operator[](entity.second->index).data;

            if (entity.first->dirty || entity.second->dirty) {
                float posX = 0, posY = 0;
                std::tie(posX, posY) = pos.worldPresent.getPos();


                int32_t minX = int32_t(col.spatialBox.minX + posX), maxX = int32_t(col.spatialBox.maxX + posX),
                minY = int32_t(col.spatialBox.minY + posY), maxY = int32_t(col.spatialBox.maxY + posY);

                if (minX < 0) minX = 0; if (minX > collisionWidth - 1) minX = collisionWidth - 1;
                if (maxX < 0) maxX = 0; if (maxX > collisionWidth - 1) maxX = collisionWidth - 1;
                if (minY < 0) minY = 0; if (minY > collisionHeight - 1) minY = collisionHeight - 1;
                if (maxY < 0) maxY = 0; if (maxY > collisionHeight - 1) maxY = collisionHeight - 1;

                size_t spatialMinX = minX / gridWidth, spatialMaxX = maxX / gridWidth, spatialMinY = minY / gridHeight, spatialMaxY = maxY / gridHeight;

                SpatialIndices& oldIndices = spatialIndices[entityIDXs[id]];

                auto collisionGroup = groupIDs[col.collisionGroup];
                if (spatialMinX != oldIndices.minX || spatialMaxX != oldIndices.maxX || spatialMinY != oldIndices.minY
                    || spatialMaxY != oldIndices.maxY) {

                    for (size_t i = 0; i < collisionGroups.size(); ++i) {
                        if (collisionGroup != i) {
                            collisionGroups[i].erase(id);
                            for (size_t j = 0; j < spatialCollisionGroups.size(); ++j) {
                                for (size_t k = 0; k < spatialCollisionGroups[i].size(); ++k) {
                                    spatialCollisionGroups[j][k][i].erase(id);
                                }
                            }
                        }
                    }

                    if (spatialMinX == spatialMaxX && spatialMinY == spatialMaxY) {
                        spatialCollisionGroups[spatialMinX][spatialMinY][collisionGroup].insert(id);
                    } else {
                        collisionGroups[collisionGroup].insert(id);
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

    auto positionPool = this->positionPool.lock();
    auto colliderPool = this->colliderPool.lock();

    for (size_t idx = 0; idx < entities.size(); ++idx) {

        const auto& entity = entities[idx];
        const Transform& position = (*positionPool)[entity.second->index].data;
        Collider& collider = (*colliderPool)[entity.first->index].data;

        float posX = 0, posY = 0;

        std::tie(posX, posY) = position.worldPresent.getPos();

        int32_t minX = int32_t(collider.spatialBox.minX + posX), maxX = int32_t(collider.spatialBox.maxX + posX),
        minY = int32_t(collider.spatialBox.minY + posY), maxY = int32_t(collider.spatialBox.maxY + posY);

        if (minX < 0) minX = 0; if (minX > collisionWidth - 1) minX = collisionWidth - 1;
        if (maxX < 0) maxX = 0; if (maxX > collisionWidth - 1) maxX = collisionWidth - 1;
        if (minY < 0) minY = 0; if (minY > collisionHeight - 1) minY = collisionHeight - 1;
        if (maxY < 0) maxY = 0; if (maxY > collisionHeight - 1) maxY = collisionHeight - 1;
        auto collisionGroup = groupIDs[collider.collisionGroup];

        size_t spatialMinX = minX / gridWidth, spatialMaxX = maxX / gridWidth, spatialMinY = minY / gridHeight, spatialMaxY = maxY / gridHeight;

        SpatialIndices& oldIndices = spatialIndices[idx];

        if (spatialMinX != oldIndices.minX || spatialMaxX != oldIndices.maxX || spatialMinY != oldIndices.minY
            || spatialMaxY != oldIndices.maxY) {

            collisionGroups[collisionGroup].erase(idxToID[idx]);

            spatialCollisionGroups[oldIndices.maxX][oldIndices.maxY][collisionGroup].erase(idxToID[idx]);

            if (spatialMinX == spatialMaxX && spatialMinY == spatialMaxY) {
                spatialCollisionGroups[spatialMinX][spatialMinY][collisionGroup].insert(idxToID[idx]);
            } else {
                collisionGroups[collisionGroup].insert(idxToID[idx]);
            }
            oldIndices.minX = spatialMinX; oldIndices.maxX = spatialMaxX;
            oldIndices.minY = spatialMinY; oldIndices.maxY = spatialMaxY;
        }

    }

    //checkPlayerCollisions();

    //checkPlayerBulletCollisions();

    for (auto &groups : collisionGraph) {
        checkCollisions(groups.first, groups.second);
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "C-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';
}

void CollisionSystem::checkCollisions(uint8_t group1ID, std::vector<uint8_t> &group2IDs) {
    auto &group1Name = groupIDToName[group1ID];

    auto positionPool = this->positionPool.lock();
    auto colliderPool = this->colliderPool.lock();

    auto& colSet1 = collisionGroups[group1ID];
    for (auto group2ID : group2IDs) {
        auto &group2Name = groupIDToName[group2ID];
        auto& colSet2 = collisionGroups[group2ID];
        //checkCollisions(colSet1, group2ID);

        for (auto id1 : colSet1) {
            Collider& col1 = (*colliderPool)[entities[entityIDXs[id1]].first->index].data;
            Transform& pos1 = (*positionPool)[entities[entityIDXs[id1]].second->index].data;
            float posX = 0, posY = 0;
            std::tie(posX, posY) = pos1.worldPresent.getPos();
            col1.position.x = posX;
            col1.position.y = posY;
            for (auto id2 : colSet2) {
                Collider& col2 = (*colliderPool)[entities[entityIDXs[id2]].first->index].data;
                Transform& pos2 = (*positionPool)[entities[entityIDXs[id2]].second->index].data;
                float posX = 0, posY = 0;
                std::tie(posX, posY) = pos2.worldPresent.getPos();
                col2.position.x = posX;
                col2.position.y = posY;
                if (collisionTable[col1.colliderType][col2.colliderType](col1, col2)) {
                    handleCollision(group1Name, group2Name, id1, id2, col1, col2);
                }
            }

            auto& spatialIndex = spatialIndices[entityIDXs[id1]];

            for (size_t i = spatialIndex.minX; i <= spatialIndex.maxX; ++i) {
                for (size_t j = spatialIndex.minY; j <= spatialIndex.maxY; ++j) {
                    auto& colSet2 = spatialCollisionGroups[i][j][group2ID];
                    for (auto id2 : colSet2) {
                        Collider& col2 = (*colliderPool)[entities[entityIDXs[id2]].first->index].data;
                        Transform& pos2 = (*positionPool)[entities[entityIDXs[id2]].second->index].data;
                        float posX = 0, posY = 0;
                        std::tie(posX, posY) = pos2.worldPresent.getPos();
                        col2.position.x = posX;
                        col2.position.y = posY;
                        if (collisionTable[col1.colliderType][col2.colliderType](col1, col2)) {
                            handleCollision(group1Name, group2Name, id1, id2, col1, col2);
                        }
                    }
                }
            }
        }


        for(size_t i = 0; i < spatialCollisionGroups.size(); ++i) {
            for (size_t j = 0; j < spatialCollisionGroups[i].size(); ++j) {
                auto& colSet1 = spatialCollisionGroups[i][j][group1ID];
                auto& colSet2 = collisionGroups[group2ID];
                //checkCollisions(colSet1, group2ID);

                for (auto id1 : colSet1) {
                    Collider& col1 = (*colliderPool)[entities[entityIDXs[id1]].first->index].data;
                    Transform& pos1 = (*positionPool)[entities[entityIDXs[id1]].second->index].data;
                    float posX = 0, posY = 0;
                    std::tie(posX, posY) = pos1.worldPresent.getPos();
                    col1.position.x = posX;
                    col1.position.y = posY;
                    for (auto id2 : colSet2) {
                        Collider& col2 = (*colliderPool)[entities[entityIDXs[id2]].first->index].data;
                        Transform& pos2 = (*positionPool)[entities[entityIDXs[id2]].second->index].data;
                        float posX = 0, posY = 0;
                        std::tie(posX, posY) = pos2.worldPresent.getPos();
                        col2.position.x = posX;
                        col2.position.y = posY;
                        if (collisionTable[col1.colliderType][col2.colliderType](col1, col2)) {
                            handleCollision(group1Name, group2Name, id1, id2, col1, col2);
                        }
                    }

                    auto& spatialIndex = spatialIndices[entityIDXs[id1]];

                    for (size_t i = spatialIndex.minX; i <= spatialIndex.maxX; ++i) {
                        for (size_t j = spatialIndex.minY; j <= spatialIndex.maxY; ++j) {
                            auto& colSet2 = spatialCollisionGroups[i][j][group2ID];
                            for (auto id2 : colSet2) {
                                Collider& col2 = (*colliderPool)[entities[entityIDXs[id2]].first->index].data;
                                Transform& pos2 = (*positionPool)[entities[entityIDXs[id2]].second->index].data;
                                float posX = 0, posY = 0;
                                std::tie(posX, posY) = pos2.worldPresent.getPos();
                                col2.position.x = posX;
                                col2.position.y = posY;
                                if (collisionTable[col1.colliderType][col2.colliderType](col1, col2)) {
                                    handleCollision(group1Name, group2Name, id1, id2, col1, col2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void CollisionSystem::handleCollision(std::string group1Name, std::string group2Name, uint32_t id1, uint32_t id2, Collider &col1, Collider &col2) {
    if (col1.collisionHandlers != sol::nil) {
        auto col1Func = static_cast<sol::table>(col1.collisionHandlers)[group2Name];
        if (col1Func != sol::nil) {
            col1Func(id2, id1);
        }
    }
    if (col2.collisionHandlers != sol::nil) {
        auto col2Func = static_cast<sol::table>(col2.collisionHandlers)[group1Name];
        if (col2Func != sol::nil) {
            col2Func(id1, id2);
        }
    }

}

void CollisionSystem::checkCollisions(CollisionSet &colSet1, uint8_t group2ID) {
    return;
    /*
    auto positionPool = this->positionPool.lock();
    auto colliderPool = this->colliderPool.lock();
    for (auto id1 : colSet1) {
        Collider& col1 = (*colliderPool)[entities[entityIDXs[id1]].first->index].data;
        Transform& pos1 = (*positionPool)[entities[entityIDXs[id1]].second->index].data;
        float posX = 0, posY = 0;
        pos1.worldPresent.getPos(posX, posY);
        col1.position.x = posX;
        col1.position.y = posY;
        auto& colSet2 = collisionGroups[group2ID];
        for (auto id2 : colSet2) {
            Collider& col2 = (*colliderPool)[entities[entityIDXs[id2]].first->index].data;
            Transform& pos2 = (*positionPool)[entities[entityIDXs[id2]].second->index].data;
            float posX = 0, posY = 0;
            pos2.worldPresent.getPos(posX, posY);
            col2.position.x = posX;
            col2.position.y = posY;
            if (collisionTable[col1.colliderType][col2.colliderType](col1, col2)) {
                std::cout << "NEW COLL\n";
            }
        }

        auto& spatialIndex = spatialIndices[entityIDXs[id1]];

        for (size_t i = spatialIndex.minX; i <= spatialIndex.maxX; ++i) {
            for (size_t j = spatialIndex.minY; j <= spatialIndex.maxY; ++j) {
                auto& colSet2 = spatialCollisionGroups[i][j][group2ID];
                for (auto id2 : colSet2) {
                    Collider& col2 = (*colliderPool)[entities[entityIDXs[id2]].first->index].data;
                    Transform& pos2 = (*positionPool)[entities[entityIDXs[id2]].second->index].data;
                    float posX = 0, posY = 0;
                    pos2.worldPresent.getPos(posX, posY);
                    col2.position.x = posX;
                    col2.position.y = posY;
                    if (collisionTable[col1.colliderType][col2.colliderType](col1, col2)) {
                        std::cout << "NEW COLL\n";
                    }
                }
            }
        }
    }
    for(size_t i = 0; i < spatialCollisionGroups.size(); ++i) {
        for (size_t j = 0; j < spatialCollisionGroups[i].size(); ++j) {
            auto& colSet2 = collisionGroups[group2ID];

            for (auto id1 : colSet1) {
                Collider& col1 = (*colliderPool)[entities[entityIDXs[id1]].first->index].data;
                Transform& pos1 = (*positionPool)[entities[entityIDXs[id1]].second->index].data;
                float posX = 0, posY = 0;
                pos1.worldPresent.getPos(posX, posY);
                col1.position.x = posX;
                col1.position.y = posY;
                for (auto id2 : colSet2) {
                    Collider& col2 = (*colliderPool)[entities[entityIDXs[id2]].first->index].data;
                    Transform& pos2 = (*positionPool)[entities[entityIDXs[id2]].second->index].data;
                    float posX = 0, posY = 0;
                    pos2.worldPresent.getPos(posX, posY);
                    col2.position.x = posX;
                    col2.position.y = posY;
                    if (collisionTable[col1.colliderType][col2.colliderType](col1, col2)) {
                        std::cout << "NEW COLL\n";
                    }

                }

                auto& spatialIndex = spatialIndices[entityIDXs[id1]];

                for (size_t i = spatialIndex.minX; i <= spatialIndex.maxX; ++i) {
                    for (size_t j = spatialIndex.minY; j <= spatialIndex.maxY; ++j) {
                        auto& colSet2 = spatialCollisionGroups[i][j][group2ID];
                        for (auto id2 : colSet2) {
                            Collider& col2 = (*colliderPool)[entities[entityIDXs[id2]].first->index].data;
                            Transform& pos2 = (*positionPool)[entities[entityIDXs[id2]].second->index].data;
                            float posX = 0, posY = 0;
                            pos2.worldPresent.getPos(posX, posY);
                            col2.position.x = posX;
                            col2.position.y = posY;
                            if (collisionTable[col1.colliderType][col2.colliderType](col1, col2)) {
                                std::cout << "NEW COLL\n";
                            }
                        }
                    }
                }
            }
        }
    }
    */
}
float dst2(const float x1, const float y1, const float x2, const float y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

bool pointToPointCollision(Collider const &p1, Collider const &p2) {
    const float xDiff = p1.position.x + p1.offset.x - p2.position.x - p2.offset.x;
    const float yDiff = p1.position.y + p1.offset.y - p2.position.y - p2.offset.y;
    /*std::cout << xDiff << '\t' << yDiff << '\n';
    if (xDiff <= 1.0f && xDiff >= -1.0f && yDiff <= 1.0f && yDiff >= -1.0f) {
        //std::cout << xDiff << '\t' << yDiff << '\n';
    }*/
    return xDiff <= 0.001f && xDiff >= -0.001f && yDiff <= 0.001f && yDiff >= -0.001f;
}

bool pointToaabbCollision(Collider const &p, Collider const &r) {
    const float posX = p.position.x + p.offset.x;
    const float posY = p.position.y + p.offset.y;
    const float minX = r.position.x + r.aabb.minX + r.offset.x;
    const float maxX = r.position.x + r.aabb.maxX + r.offset.x;
    const float minY = r.position.y + r.aabb.minY + r.offset.y;
    const float maxY = r.position.y + r.aabb.maxY + r.offset.y;
    return (posX >= minX && posX <= maxX
        && posY >= minY && posY <= maxY)
        || ( ( (minX >= posX && minX <= posX + 1) || (maxX >= posX && maxX <= posX + 1) )
        && ( (minY >= posY && minY <= posY + 1) || (maxY >= posY && maxY <= posY + 1) ) );
}

bool pointToCircleCollision(Collider const &p, Collider const &c) {
    const float posX = p.position.x + p.offset.x;
    const float posY = p.position.y + p.offset.y;
    const float centerX = c.position.x + c.offset.x;
    const float centerY = c.position.y + c.offset.y;
    return dst2(posX, posY, centerX, centerY) <= (c.circle.radius * c.circle.radius);
}

bool aabbToaabbCollision(Collider const &r1, Collider const &r2) {
    const float aabbWidthGrowth = (r2.aabb.maxX - r2.aabb.minX) / 2;
    const float aabbHeightGrowth = (r2.aabb.maxY - r2.aabb.minY) / 2;
    const float minX = r1.position.x + r1.aabb.minX + r1.offset.x - aabbWidthGrowth;
    const float maxX = r1.position.x + r1.aabb.maxX + r1.offset.x + aabbWidthGrowth;
    const float minY = r1.position.y + r1.aabb.minY + r1.offset.y - aabbHeightGrowth;
    const float maxY = r1.position.y + r1.aabb.maxY + r1.offset.y + aabbHeightGrowth;
    const float posX = r2.position.x + r2.offset.x;
    const float posY = r2.position.y + r2.offset.y;
    return (posX >= minX && posX <= maxX
        && posY >= minY && posY <= maxY)
        || ( ( (minX >= posX && minX <= posX + 1) || (maxX >= posX && maxX <= posX + 1) )
        && ( (minY >= posY && minY <= posY + 1) || (maxY >= posY && maxY <= posY + 1) ) );
}

bool aabbToPointCollision(Collider const &r1, Collider const &p2) {
    return pointToaabbCollision(p2, r1);
}

bool aabbToCircleCollision(Collider const &r, Collider const &c) {
    const float minX = r.position.x + r.aabb.minX + r.offset.x;
    const float maxX = r.position.x + r.aabb.maxX + r.offset.x;
    const float minY = r.position.y + r.aabb.minY + r.offset.y;
    const float maxY = r.position.y + r.aabb.maxY + r.offset.y;
    const float rad2 = c.circle.radius * c.circle.radius;
    const float centerX = c.position.x + c.offset.x;
    const float centerY = c.position.y + c.offset.y;
    return dst2(minX, minY, centerX, centerY) <= rad2
        || dst2(maxX, minY, centerX, centerY) <= rad2
        || dst2(minX, maxY, centerX, centerY) <= rad2
        || dst2(maxX, maxY, centerX, centerY) <= rad2;
}

bool circleToCircleCollision(Collider const &c1, Collider const &c2) {
    const float centerX1 = c1.position.x + c1.offset.x;
    const float centerY1 = c1.position.y + c1.offset.y;
    const float centerX2 = c2.position.x + c2.offset.x;
    const float centerY2 = c2.position.y + c2.offset.y;
    const float rad2 = (c1.circle.radius + c2.circle.radius) * (c1.circle.radius + c2.circle.radius);
    return dst2(centerX1, centerY1, centerX2, centerY2) <= rad2;
}

bool circleToPointCollision(Collider const &c, Collider const &p) {
    return pointToCircleCollision(p, c);
}

bool circleToaabbCollision(Collider const &c, Collider const &r) {
    return aabbToCircleCollision(r, c);
}
