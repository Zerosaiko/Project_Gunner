#include "SpawnSystem.h"
#include "SDL.h"
#include "playerComponents.h"
#include <cmath>

static const float pi = acos(-1.0f);

SpawnSystem::SpawnSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    spawnPool = manager->getComponentPool<Component<Spawner::name, Spawner>>();
    transformPool = manager->getComponentPool<Component<Transform::name, Transform>>();
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
    auto entity = manager->getEntity(id);
    if (entity) {
        const auto &components = entity->components;
        auto spawner = components.find("spawner");
        auto transform = components.find("transform");
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay == components.end() || !delay->second.active) && (pause == components.end() || !pause->second.active)
            && spawner != components.end() && spawner->second.active && transform->second.active && transform->second.active) {
            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&spawner->second, &transform->second);
            Spawner spawnCmp = (*spawnPool.lock())[spawner->second.index].data;
            std::vector<EntityPreallocationInfo> preAllocDat;
            for (uint32_t i = 0; i < spawnCmp.bursts.size(); ++i) {
                preAllocDat.emplace_back(spawnCmp.bursts[i].spawnsPerRun);
                preAllocDat.back().data = spawnCmp.bursts[i].addComponents;

                preAllocDat.back().idList.resize(spawnCmp.bursts[i].spawnsPerRun, 0u);
                totalSpawnCount += spawnCmp.bursts[i].spawnsPerRun;
            }
            preAllocationData.emplace_back(std::move(preAllocDat));
        }
    }
}

void SpawnSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    for (auto &preAlloc : preAllocationData[entityIDXs[id]]) {
        for (std::size_t i = 0; i < preAlloc.dataIdx; i++) {
            manager->destroyEntity(preAlloc.idList[i]);
        }
        totalSpawnCount -= preAlloc.currentSpawnCount;
    }
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    preAllocationData[entityIDXs[id]] = std::move(preAllocationData.back());
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
        const auto &components = fullEntity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay != components.end() && delay->second.active) || (pause != components.end() && pause->second.active) ) {
            removeEntity(id);
        } else if (entity.first->dirty) {
            Spawner& spawnCmp = (*spawnPool.lock())[entity.first->index].data;
            spawnCmp.currentBurst = 0;
            for (auto &preAlloc : preAllocationData[entityIDXs[id]]) {
                for (std::size_t i = 0; i < preAlloc.dataIdx; i++) {
                    manager->destroyEntity(preAlloc.idList[i]);
                }
                totalSpawnCount -= preAlloc.currentSpawnCount;
            }
            preAllocationData[entityIDXs[id]].resize(spawnCmp.bursts.size(), EntityPreallocationInfo(0));
            for (size_t i = 0; i < spawnCmp.bursts.size(); ++i) {
                auto& preAllocDat = preAllocationData[entityIDXs[id]];
                preAllocDat[i].data = spawnCmp.bursts[i].addComponents;
                preAllocDat[i].idList.resize(spawnCmp.bursts[i].spawnsPerRun, 0u);
                preAllocDat[i].dataIdx = 0u;
                preAllocDat[i].currentSpawnCount = spawnCmp.bursts[i].spawnsPerRun;
                totalSpawnCount += spawnCmp.bursts[i].spawnsPerRun;
            }
        }
    }
}

void SpawnSystem::process(float dt) {

    dt *= 1000.0f;

    bool spawned = false;

    auto startT = SDL_GetPerformanceCounter();
    const auto playerGroupPtr = manager->groupManager.getIDGroup("player");

    auto transformPool = this->transformPool.lock();
    auto spawnPool = this->spawnPool.lock();

    float aggroPlayerX = 0, aggroPlayerY = 0;
    if (playerGroupPtr) {
        Transform aggroPlayerPos;
        const auto& playerGroup = *playerGroupPtr;
        const auto playerPool = manager->getComponentPool<Component<PlayerCmp::name, PlayerCmp>>().lock();
        float highestAggro = -1.0f;
        for (const auto& id : playerGroup) {
            auto entity = manager->getEntity(id);
            const auto &components = entity->components;
            const auto& playerCmp = components.find("player");
            if (playerCmp != components.end()) {
                PlayerCmp& player = playerPool->operator[](playerCmp->second.index).data;
                if (player.aggro > highestAggro) {
                    highestAggro = player.aggro;
                    const auto& posCmp = components.find("transform");
                    if (posCmp != components.end()) {
                        aggroPlayerPos = transformPool->operator[](posCmp->second.index).data;
                        std::tie(aggroPlayerX, aggroPlayerY) = aggroPlayerPos.worldPresent.getPos();
                    }
                }

            }
        }

    }

    for(size_t i = 0; i < entities.size(); ++i) {
        const auto& entity = entities[i];
        const Transform& position = (*transformPool)[entity.second->index].data;

        float positionX = 0, positionY = 0;
        std::tie(positionX, positionY) = position.worldPresent.getPos();
        Spawner& spawner = (*spawnPool)[entity.first->index].data;
        Spawner::Burst* burst = &spawner.bursts[spawner.currentBurst];

        spawner.currentTime += dt;
        while (spawner.currentTime >= burst->repeatRate) {
            EntityPreallocationInfo& preAlloc = preAllocationData[i][spawner.currentBurst];
            spawned = true;
            auto spawnPositions = burst->position;

            auto spawnVelocities = burst->velocity;
            for (uint32_t j = 0; j < burst->spawnsPerRun; ++j) {
                Position newPos, adjPos;
                Velocity newVel;

                newPos.posX = newPos.pastPosX = adjPos.posX = 0.0f;
                newPos.posY = newPos.pastPosY = adjPos.posY = 0.0f;

                newVel.velX = 0.0f;
                newVel.velY = 0.0f;

                switch (burst->spawnPosition) {
                default: break;

                case Spawner::SpawnPos::AlongList :
                    newPos.posX = burst->spawnPoints.at((j % burst->spawnsPerRun) * 2);
                    newPos.posY = burst->spawnPoints.at((j % burst->spawnsPerRun) * 2 + 1);

                    break;
                }

                switch (burst->posDirection) {

                case Spawner::PointStyle::XY :
                    newPos.posX += spawnPositions.xyVec.x;
                    newPos.posY += spawnPositions.xyVec.y;
                    adjPos = newPos;
                    if (burst->relative == Spawner::Relative::Source) {
                        newPos.posX += positionX;
                        newPos.posY += positionY;
                    } else if (burst->relative == Spawner::Relative::Player) {
                        // needs to be handled
                    }
                    newPos.pastPosX = newPos.posX;
                    newPos.pastPosY = newPos.posY;
                    spawnPositions.xyVec.x += burst->position.xyVec.dx;
                    spawnPositions.xyVec.y += burst->position.xyVec.dy;
                    break;

                case Spawner::PointStyle::Rad :
                    newPos.posX += cosf(spawnPositions.dirSpd.direction) * fabsf(spawnPositions.dirSpd.speed);
                    newPos.posY -= sinf(spawnPositions.dirSpd.direction) * fabsf(spawnPositions.dirSpd.speed);
                    adjPos = newPos;
                    if (burst->relative == Spawner::Relative::Source) {
                        newPos.posX += positionX;

                        newPos.posY += positionY;
                    } else if (burst->relative == Spawner::Relative::Player) {
                        newPos.posX += aggroPlayerX;
                        newPos.posY += aggroPlayerY;
                    }
                    newPos.pastPosX = newPos.posX;
                    newPos.pastPosY = newPos.posY;
                    spawnPositions.dirSpd.direction += burst->position.dirSpd.deltaDirection;
                    spawnPositions.dirSpd.speed += burst->position.dirSpd.dSpeed;
                    break;

                default :
                    newPos.pastPosX = newPos.posX = 0.f;
                    newPos.pastPosY = newPos.posY = 0.f;

                }

                switch (burst->spawnVelocity) {

                case Spawner::SpawnVel::Default :

                    break;

                case Spawner::SpawnVel::Aimed :
                    newVel.velX = aggroPlayerX - newPos.posX;
                    newVel.velY = aggroPlayerY - newPos.posY;
                    {
                        float length = sqrtf( (newVel.velX * newVel.velX + newVel.velY * newVel.velY) );
                        if (length != 0.0f)
                            newVel.velX /= length; newVel.velY /= length;
                    }
                    // needs code to aim at player now that position is known
                    break;

                case Spawner::SpawnVel::AwayFromPlayer :
                    newVel.velX = newPos.posX - aggroPlayerX;
                    newVel.velY = newPos.posY - aggroPlayerY;
                    {
                        float length = sqrtf( (newVel.velX * newVel.velX + newVel.velY * newVel.velY) );
                        if (length != 0.0f)
                            newVel.velX /= length; newVel.velY /= length;
                    }

                    // needs code to aim at player now that position is known
                    break;

                case Spawner::SpawnVel::AimedBySource :
                    newVel.velX = aggroPlayerX - positionX;
                    newVel.velY = aggroPlayerY - positionY;
                    {
                        float length = sqrtf( (newVel.velX * newVel.velX + newVel.velY * newVel.velY) );
                        if (length != 0.0f)
                            newVel.velX /= length; newVel.velY /= length;
                    }
                    // needs code to aim at player now that position is known
                    break;

                case Spawner::SpawnVel::AimedAwayBySource :
                    newVel.velX = positionX - aggroPlayerX;
                    newVel.velY = positionY - aggroPlayerY;
                    {
                        float length = sqrtf( (newVel.velX * newVel.velX + newVel.velY * newVel.velY) );
                        if (length != 0.0f)
                            newVel.velX /= length; newVel.velY /= length;
                    }
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
                    newVel.velX = burst->velocityList.at((j % burst->spawnsPerRun) * 2);
                    newVel.velY = burst->velocityList.at((j % burst->spawnsPerRun) * 2 + 1);

                    break;

                default:
                    break;

                }

                switch (burst->velDirection) {

                case Spawner::PointStyle::XY :
                    newVel.velX = spawnVelocities.xyVec.x;
                    newVel.velY = spawnVelocities.xyVec.y;
                    spawnVelocities.xyVec.x += burst->velocity.xyVec.dx;
                    spawnVelocities.xyVec.y += burst->velocity.xyVec.dy;
                    break;

                case Spawner::PointStyle::Rad :
                    newVel.velX = cosf(spawnVelocities.dirSpd.direction) * fabsf(spawnVelocities.dirSpd.speed);
                    newVel.velY = -sinf(spawnVelocities.dirSpd.direction) * fabsf(spawnVelocities.dirSpd.speed);
                    spawnVelocities.dirSpd.direction += burst->velocity.dirSpd.deltaDirection;
                    spawnVelocities.dirSpd.speed += burst->velocity.dirSpd.dSpeed;
                    break;

                case Spawner::PointStyle::Speed :
                    newVel.velX *= spawnVelocities.speed.current;
                    newVel.velY *= spawnVelocities.speed.current;

                    spawnVelocities.speed.current += spawnVelocities.speed.delta;

                    break;

                default: break;
                }

                uint32_t newEntityID = 0;
                if (j >= preAlloc.dataIdx) {
                    newEntityID = manager->createEntity();
                    if (burst->addComponents.size() > 0) {
                        for ( sol::table cmp : preAlloc.data[j % preAlloc.data.size()] ) {
                            std::string cmpName = cmp["componentName"];
                            manager->addComponent(cmpName,
                                cmp["component"], newEntityID);
                        }
                        --totalSpawnCount;
                        --preAlloc.currentSpawnCount;
                    }

                } else {
                    newEntityID = preAlloc.idList[j];
                }
                Transform tf;
                tf.local.translate(newPos.posX, newPos.posY);
                tf.hasParent = false;
                if (burst->rotate) {
                    tf.local.rotate(-(atan2(newVel.velY, newVel.velX) * 180 / pi));
                }
                Component<Transform::name, Transform> posComponent{tf};
                Component<Velocity::name, Velocity> velComponent{newVel};
                manager->addComponent(posComponent, newEntityID);
                manager->addComponent(velComponent, newEntityID);

                manager->forceRefresh(preAlloc.idList[j]);
                preAlloc.idList[j] = 0;

            }

            switch (burst->posDirection) {

            case Spawner::PointStyle::XY :
                burst->position.xyVec.x += burst->position.xyVec.persistDx;
                burst->position.xyVec.y += burst->position.xyVec.persistDy;
                break;

            case Spawner::PointStyle::Rad :
                burst->position.dirSpd.direction += burst->position.dirSpd.persistDeltaDirection;
                burst->position.dirSpd.speed += burst->position.dirSpd.persistDSpeed;
                break;

            default : break;

            }

            switch (burst->velDirection) {

            case Spawner::PointStyle::XY :
                burst->velocity.xyVec.x += burst->velocity.xyVec.persistDx;
                burst->velocity.xyVec.y += burst->velocity.xyVec.persistDy;
                break;

            case Spawner::PointStyle::Rad :
                burst->velocity.dirSpd.direction += burst->velocity.dirSpd.persistDeltaDirection;
                burst->velocity.dirSpd.speed += burst->velocity.dirSpd.persistDSpeed;
                break;

            case Spawner::PointStyle::Speed :
                burst->velocity.speed.current += burst->velocity.speed.persistDelta;
                break;
            }
            preAlloc.dataIdx = 0;
            totalSpawnCount += burst->spawnsPerRun;
            preAlloc.currentSpawnCount += burst->spawnsPerRun;
            spawner.currentTime -= burst->repeatRate;
            if (burst->runs < burst->runCount) {
                ++burst->runs;
                if (burst->runs >= burst->runCount){
                    burst->runs = 0;
                    ++spawner.currentBurst;
                    spawner.currentBurst %= spawner.bursts.size();
                    burst = &spawner.bursts[spawner.currentBurst];
                    spawner.currentTime = burst->repeatRate - burst->initialDelay;
                    if (spawner.totalRunCount > 0 && spawner.currentBurst == 0 && --spawner.totalRunCount == 0) {
                        manager->removeComponent<Spawner>(idxToID[i]);
                        break;
                    }
                }
            }
        }
    }
    auto endT = SDL_GetPerformanceCounter();
    // if spawning hasn't taken above a certain amount of time, preallocate until the time limit is reached
    const float timeLimit = 3.0f;
    float timeTaken = (1000.f / SDL_GetPerformanceFrequency() * (endT - startT));
    bool preAllocated = false;

    auto fullPreAlloT = SDL_GetPerformanceCounter();
    auto cpyCount = totalSpawnCount;

    while ( totalSpawnCount && timeTaken < timeLimit ) {

        preAllocated = true;
        auto preAlloT = SDL_GetPerformanceCounter();

        if (preAlloIdx >= preAllocationData.size()) preAlloIdx = 0;

        std::vector<EntityPreallocationInfo>& preAllocVec = preAllocationData[preAlloIdx];
        for (size_t i = 0; i < preAllocVec.size() && timeTaken < timeLimit; ++i) {
            auto &preAlloc = preAllocVec[i];
            if (preAlloc.dataIdx < preAlloc.idList.size()) {
                size_t idx = preAlloc.dataIdx % preAlloc.data.size();
                preAlloc.idList[preAlloc.dataIdx] = manager->createEntity();
                for ( sol::table cmp : preAlloc.data[idx] ) {
                    std::string cmpName = cmp["componentName"];
                    manager->addComponent(cmpName,
                        cmp["component"], preAlloc.idList[preAlloc.dataIdx]);
                }
                --totalSpawnCount;
                --preAlloc.currentSpawnCount;
                manager->excludeFromRefresh(preAlloc.idList[preAlloc.dataIdx]);
                ++preAlloc.dataIdx;
            }

            if (preAlloc.dataIdx >= preAlloc.idList.size()) {
                ++preAlloIdx;
            }
            auto postAlloT = SDL_GetPerformanceCounter();
            timeTaken += (1000.f / SDL_GetPerformanceFrequency() * (postAlloT - preAlloT));
        }

    }

    auto fullPostAlloT = SDL_GetPerformanceCounter();



    if (spawned && false)
        std::cout << "SPAWN - " << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';
    if (preAllocated && false)
        std::cout << "PRE ALLOC - " << (cpyCount - totalSpawnCount)
            << " out of " << cpyCount << " in " << (1000.f / SDL_GetPerformanceFrequency() * (fullPostAlloT - fullPreAlloT)) << '\n';

}

SpawnSystem::EntityPreallocationInfo::EntityPreallocationInfo(size_t spawnCnt)
    : dataIdx(0), currentSpawnCount(spawnCnt) {}


