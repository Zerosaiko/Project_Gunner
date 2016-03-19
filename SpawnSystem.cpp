#include "SpawnSystem.h"
#include "SDL.h"
#include <cmath>

SpawnSystem::SpawnSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    spawnPool = manager->getComponentPool<Component<Spawner::name, Spawner>>();
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
    preAlloIdx = 0;
    totalSpawnCount = 0;
};

void SpawnSystem::initialize() {}

void SpawnSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto spawner = entity->find("spawner");
        auto position = entity->find("position");
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ( (delay == entity->end() || !delay->second.active) && (pause == entity->end() || !pause->second.active)
            && spawner != entity->end() && position != entity->end() && spawner->second.active && position->second.active) {
            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&spawner->second, &position->second);

            Spawner& spawnCmp = (*spawnPool)[spawner->second.index].data;
            preAllocationData.emplace_back(spawnCmp.spawnsPerRun);
            preAllocationData.back().data.resize(spawnCmp.addComponents.size());

            for (size_t i = 0; i < spawnCmp.addComponents.size(); ++i) {
                for (size_t j = 0; j < spawnCmp.addComponents[i].size(); ++j) {
                    preAllocationData.back().data[i].emplace_back(manager->tokenize(spawnCmp.addComponents[i][j]));
                }
            }
            for(size_t k = 0; k < spawnCmp.spawnsPerRun; ++k) {
                preAllocationData.back().idList.emplace_back(0);
                preAllocationData.back().isAllocated.emplace_back(false);
            }
            totalSpawnCount += spawnCmp.spawnsPerRun;
        }
    }
}

void SpawnSystem::removeEntity(uint32_t id) {
    if (!hasEntity[id]) return;
    for (size_t i = 0; i < preAllocationData[entityIDXs[id]].idList.size(); ++i) {
        if (preAllocationData[entityIDXs[id]].isAllocated[i]) {
            manager->destroyEntity(preAllocationData[entityIDXs[id]].idList[i]);
        }
    }
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    totalSpawnCount -= preAllocationData[entityIDXs[id]].currentSpawnCount;
    preAllocationData[entityIDXs[id]] = preAllocationData.back();
    preAllocationData.pop_back();
    hasEntity[id] = false;
}

void SpawnSystem::refreshEntity(uint32_t id) {
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
            Spawner& spawnCmp = (*spawnPool)[entity.first->index].data;
            for (auto& entityID : preAllocationData[entityIDXs[id]].idList) {
                manager->destroyEntity(entityID);
            }
            totalSpawnCount -= preAllocationData[entityIDXs[id]].currentSpawnCount;
            totalSpawnCount += spawnCmp.spawnsPerRun;
            preAllocationData[entityIDXs[id]].idList.clear();
            preAllocationData[entityIDXs[id]].idList.resize(spawnCmp.spawnsPerRun, 0);
            preAllocationData[entityIDXs[id]].isAllocated.clear();
            preAllocationData[entityIDXs[id]].isAllocated.resize(spawnCmp.spawnsPerRun, false);
            preAllocationData[entityIDXs[id]].data.clear();
            preAllocationData[entityIDXs[id]].data.resize(spawnCmp.addComponents.size());

            for (size_t i = 0; i < spawnCmp.addComponents.size(); ++i) {
                for (size_t j = 0; j < spawnCmp.addComponents[i].size(); ++j) {
                    preAllocationData[entityIDXs[id]].data[i].emplace_back(manager->tokenize(spawnCmp.addComponents[i][j]));
                }
            }
        }
    }
}

void SpawnSystem::process(float dt) {
    dt *= 1000.0f;

    bool spawned = false;

    auto startT = SDL_GetPerformanceCounter();

    for(size_t i = 0; i < entities.size(); ++i) {
        const auto& entity = entities[i];
        const Position& position = (*positionPool)[entity.second->index].data;
        Spawner& spawner = (*spawnPool)[entity.first->index].data;
        spawner.currentTime += dt;
        while (spawner.currentTime >= spawner.repeatRate) {
            spawned = true;
            Spawner::Position spawnPositions = spawner.position;

            Spawner::Velocity spawnVelocities = spawner.velocity;

            switch (spawner.posDirection) {

            case Spawner::PointStyle::XY :
                spawner.position.xyVec.x += spawner.position.xyVec.persistDx;
                spawner.position.xyVec.y += spawner.position.xyVec.persistDy;
                break;

            case Spawner::PointStyle::Rad :
                spawner.position.dirSpd.direction += spawner.position.dirSpd.persistDeltaDirection;
                spawner.position.dirSpd.speed += spawner.position.dirSpd.persistDSpeed;
                break;

            default : break;

            }

            switch (spawner.velDirection) {

            case Spawner::PointStyle::XY :
                spawner.velocity.xyVec.x += spawner.velocity.xyVec.persistDx;
                spawner.velocity.xyVec.y += spawner.velocity.xyVec.persistDy;
                break;

            case Spawner::PointStyle::Rad :
                spawner.velocity.dirSpd.direction += spawner.velocity.dirSpd.persistDeltaDirection;
                spawner.velocity.dirSpd.speed += spawner.velocity.dirSpd.persistDSpeed;
                break;

            case Spawner::PointStyle::Speed :
                spawner.velocity.speed.current += spawner.velocity.speed.persistDelta;
                break;
            }

            for (uint32_t j = 0; j < spawner.spawnsPerRun; ++j) {

                Position newPos, adjPos;
                Velocity newVel;

                newPos.posX = newPos.pastPosX = adjPos.posX = 0.0f;
                newPos.posY = newPos.pastPosY = adjPos.posY = 0.0f;

                newVel.velX = 0.0f;
                newVel.velY = 0.0f;

                switch (spawner.spawnPosition) {
                default: break;

                case Spawner::SpawnPos::AlongList :
                    newPos.posX = spawner.spawnPoints.at((j % spawner.spawnsPerRun) * 2);
                    newPos.posY = spawner.spawnPoints.at((j % spawner.spawnsPerRun) * 2 + 1);

                    break;
                }

                switch (spawner.posDirection) {

                case Spawner::PointStyle::XY :
                    newPos.posX += spawnPositions.xyVec.x;
                    newPos.posY += spawnPositions.xyVec.y;
                    adjPos = newPos;
                    if (spawner.relative == Spawner::Relative::Source) {
                        newPos.posX += position.posX;
                        newPos.posY += position.posY;
                    } else if (spawner.relative == Spawner::Relative::Player) {
                        // needs to be handled
                    }
                    newPos.pastPosX = newPos.posX;
                    newPos.pastPosY = newPos.posY;
                    spawnPositions.xyVec.x += spawner.position.xyVec.dx;
                    spawnPositions.xyVec.y += spawner.position.xyVec.dy;
                    break;

                case Spawner::PointStyle::Rad :
                    newPos.posX += cosf(spawnPositions.dirSpd.direction) * fabsf(spawnPositions.dirSpd.speed);
                    newPos.posY += sinf(spawnPositions.dirSpd.direction) * fabsf(spawnPositions.dirSpd.speed);
                    adjPos = newPos;
                    if (spawner.relative == Spawner::Relative::Source) {
                        newPos.posX += position.posX;

                        newPos.posY += position.posY;
                    } else if (spawner.relative == Spawner::Relative::Player) {
                        // needs to be handled
                    }
                    newPos.pastPosX = newPos.posX;
                    newPos.pastPosY = newPos.posY;
                    spawnPositions.dirSpd.direction += spawner.position.dirSpd.deltaDirection;
                    spawnPositions.dirSpd.speed += spawner.position.dirSpd.dSpeed;
                    break;

                default :
                    newPos.pastPosX = newPos.posX = 0.f;
                    newPos.pastPosY = newPos.posY = 0.f;

                }

                switch (spawner.spawnVelocity) {

                case Spawner::SpawnVel::Default :

                    break;

                case Spawner::SpawnVel::Aimed :
                    // needs code to aim at player now that position is known
                    break;

                case Spawner::SpawnVel::AwayFromPlayer :
                    // needs code to aim at player now that position is known
                    break;

                case Spawner::SpawnVel::TowardOrigin :
                    newVel.velX = -adjPos.posX;
                    newVel.velY = -adjPos.posY;
                    {
                        float length = sqrtf( (newVel.velX * newVel.velX + newVel.velY * newVel.velY) );
                        if (length != 0.0f)
                            newVel.velX /= length; newVel.velY /= length;
                    }

                    break;

                case Spawner::SpawnVel::AwayFromOrigin :
                    newVel.velX = adjPos.posX;
                    newVel.velY = adjPos.posY;
                    {
                        float length = sqrtf( (newVel.velX * newVel.velX + newVel.velY * newVel.velY) );
                        if (length != 0.0f)
                            newVel.velX /= length; newVel.velY /= length;
                    }
                    break;

                case Spawner::SpawnVel::UseList :
                    newVel.velX = spawner.velocityList.at((j % spawner.spawnsPerRun) * 2);
                    newVel.velY = spawner.velocityList.at((j % spawner.spawnsPerRun) * 2 + 1);

                    break;

                default:
                    break;

                }

                switch (spawner.velDirection) {

                case Spawner::PointStyle::XY :
                    newVel.velX = spawnVelocities.xyVec.x;
                    newVel.velY = spawnVelocities.xyVec.y;
                    spawnVelocities.xyVec.x += spawner.velocity.xyVec.dx;
                    spawnVelocities.xyVec.y += spawner.velocity.xyVec.dy;
                    break;

                case Spawner::PointStyle::Rad :
                    newVel.velX = cosf(spawnVelocities.dirSpd.direction) * fabsf(spawnVelocities.dirSpd.speed);
                    newVel.velY = sinf(spawnVelocities.dirSpd.direction) * fabsf(spawnVelocities.dirSpd.speed);
                    spawnVelocities.dirSpd.direction += spawner.velocity.dirSpd.deltaDirection;
                    spawnVelocities.dirSpd.speed += spawner.velocity.dirSpd.dSpeed;
                    break;

                case Spawner::PointStyle::Speed :
                    newVel.velX *= spawnVelocities.speed.current;
                    newVel.velY *= spawnVelocities.speed.current;

                    spawnVelocities.speed.current += spawnVelocities.speed.delta;

                    break;

                default: break;
                }

                EntityPreallocationInfo& preAlloc = preAllocationData[i];
                uint32_t newEntityID = 0;
                newEntityID = preAlloc.idList[j];
                if (!preAlloc.isAllocated[j]) {
                    newEntityID = manager->createEntity();
                }

                Component<Position::name, Position> posComponent{newPos};
                Component<Velocity::name, Velocity> velComponent{newVel};
                manager->addComponent<Component<Position::name, Position>>(posComponent, newEntityID);
                manager->addComponent<Component<Velocity::name, Velocity>>(velComponent, newEntityID);

                if (spawner.addComponents.size() > 0 && !preAlloc.isAllocated[j]) {
                    auto idx = j % spawner.addComponents.size();
                    for ( std::vector<std::string>& cmp : preAlloc.data.at(idx) ) {
                        manager->addComponent(cmp, newEntityID);
                    }
                    --totalSpawnCount;
                    --preAlloc.currentSpawnCount;


                }
                manager->forceRefresh(preAlloc.idList[j]);
                preAlloc.idList[j] = 0;
                preAlloc.isAllocated[j] = false;
                preAlloc.dataIdx = 0;

            }
            totalSpawnCount += spawner.spawnsPerRun;
            preAllocationData[i].currentSpawnCount += spawner.spawnsPerRun;
            spawner.currentTime -= spawner.repeatRate;
            if (spawner.runCount > 0) {
                --spawner.runCount;
                if (spawner.runCount == 0) manager->removeComponent<Spawner>(idxToID[i]);;
            }
        }
    }

    // if spawning hasn't taken above a certain amount of time, preallocate until the time limit is reached

    float timeLimit = (1000.f / SDL_GetPerformanceFrequency() * (SDL_GetPerformanceCounter() - startT));
    bool preAllocated = false;
    auto fullPreAlloT = SDL_GetPerformanceCounter();
    auto cpyCount = totalSpawnCount;
    while ( totalSpawnCount && timeLimit < 3.0f ) {

        preAllocated = true;
        auto preAlloT = SDL_GetPerformanceCounter();

        if (preAlloIdx >= preAllocationData.size()) preAlloIdx = 0;

        EntityPreallocationInfo& preAlloc = preAllocationData[preAlloIdx];
        if (preAlloc.dataIdx < preAlloc.idList.size() && !preAlloc.isAllocated[preAlloc.dataIdx]) {
            size_t idx = preAlloc.dataIdx % preAlloc.data.size();
            preAlloc.idList[preAlloc.dataIdx] = manager->createEntity();
            for ( std::vector<std::string>& cmp : preAlloc.data.at(idx) ) {
                manager->addComponent(cmp, preAlloc.idList[preAlloc.dataIdx]);
            }
            --totalSpawnCount;
            --preAlloc.currentSpawnCount;
            preAlloc.isAllocated[preAlloc.dataIdx] = true;
            manager->excludeFromRefresh(preAlloc.idList[preAlloc.dataIdx]);
        }
        ++preAlloc.dataIdx;

        if (preAlloc.dataIdx >= preAlloc.idList.size()) {
            preAlloc.dataIdx = 0;
            ++preAlloIdx;
        }
        auto postAlloT = SDL_GetPerformanceCounter();
        timeLimit += (1000.f / SDL_GetPerformanceFrequency() * (postAlloT - preAlloT));

    }
    decltype(startT) fullPostAlloT;

    auto endT = fullPostAlloT = SDL_GetPerformanceCounter();

    if (spawned)
        std::cout << "SPAWN - " << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';
    if (preAllocated)
        std::cout << "PRE ALLOC - " << (cpyCount - totalSpawnCount)
            << " out of " << cpyCount << " in " << (1000.f / SDL_GetPerformanceFrequency() * (fullPostAlloT - fullPreAlloT)) << '\n';

}

SpawnSystem::EntityPreallocationInfo::EntityPreallocationInfo(size_t spawnCnt)
    : dataIdx(0), currentSpawnCount(spawnCnt) {}
