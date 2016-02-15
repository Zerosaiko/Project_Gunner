#include "SpawnSystem.h"
#include "SDL.h"
#include <cmath>

SpawnSystem::SpawnSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    spawnPool = manager->getComponentPool<Component<Spawner::name, Spawner>>();
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
};

void SpawnSystem::initialize() {}

void SpawnSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto spawner = entity->find("spawner");
            auto position = entity->find("position");
            if (spawner != entity->end() && position != entity->end() && spawner->second.first && position->second.first) {
                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&spawner->second, &position->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    entities[idx] = make_pair(&spawner->second, &position->second);
                }
            }
        }
    }
}

void SpawnSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    }
}

void SpawnSystem::refreshEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !(entities[entityID->second].first->first && entities[entityID->second].second->first )) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID == entityIDs.end() ) {
        addEntity(id);
    }

}

void SpawnSystem::process(float dt) {
    dt *= 1000.0f;

    auto startT = SDL_GetPerformanceCounter();

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Spawner& spawner = (*spawnPool)[entity.first->second].data;
        spawner.currentTime += dt;
        while (spawner.currentTime >= spawner.repeatRate) {
            Spawner::Position spawnPositions;
            Spawner::Velocity spawnVelocities;

            switch (spawner.posDirection) {

            case Spawner::PointStyle::XY :
                spawnPositions.xyVec = spawner.position.xyVec;
                break;

            case Spawner::PointStyle::Rad :
                spawnPositions.dirSpd = spawner.position.dirSpd;
                break;

            default : break;

            }

            switch (spawner.velDirection) {

            case Spawner::PointStyle::XY :
                spawnVelocities.xyVec = spawner.velocity.xyVec;
                break;

            case Spawner::PointStyle::Rad :
                spawnVelocities.dirSpd = spawner.velocity.dirSpd;
                break;

            case Spawner::PointStyle::Speed :
                spawnVelocities.speed = spawner.velocity.speed;
                break;
            }

            for (uint32_t i = 0; i < spawner.spawnsPerRun; ++i) {
                const Position& position = (*positionPool)[entity.second->second].data;
                uint32_t newEntityID = manager->createEntity();

                Position newPos, adjPos;
                Velocity newVel;

                newPos.posX = newPos.pastPosX = 0.0f;
                newPos.posY = newPos.pastPosY = 0.0f;

                newVel.velX = 0.0f;
                newVel.velY = 0.0f;

                switch (spawner.spawnPosition) {
                default: break;

                case Spawner::SpawnPos::AlongList :
                    newPos.posX = spawner.spawnPoints.at((i % spawner.spawnsPerRun) * 2);
                    newPos.posY = spawner.spawnPoints.at((i % spawner.spawnsPerRun) * 2 + 1);

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
                    newPos.posX += cosf(spawnPositions.dirSpd.direction) * spawnPositions.dirSpd.speed;
                    newPos.posY += sinf(spawnPositions.dirSpd.direction) * spawnPositions.dirSpd.speed;
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
                        newVel.velX /= length; newVel.velY /= length;
                    }

                    break;

                case Spawner::SpawnVel::AwayFromOrigin :
                    newVel.velX = adjPos.posX;
                    newVel.velY = adjPos.posY;
                    {
                        float length = sqrtf( (newVel.velX * newVel.velX + newVel.velY * newVel.velY) );
                        newVel.velX /= length; newVel.velY /= length;
                    }
                    break;

                case Spawner::SpawnVel::UseList :
                    newVel.velX = spawner.velocityList.at((i % spawner.spawnsPerRun) * 2);
                    newVel.velY = spawner.velocityList.at((i % spawner.spawnsPerRun) * 2 + 1);

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
                    newVel.velX = cosf(spawnVelocities.dirSpd.direction) * spawnVelocities.dirSpd.speed;
                    newVel.velY = sinf(spawnVelocities.dirSpd.direction) * spawnVelocities.dirSpd.speed;
                    spawnVelocities.dirSpd.direction += spawner.velocity.dirSpd.deltaDirection;
                    spawnVelocities.dirSpd.speed += spawner.velocity.dirSpd.dSpeed;
                    break;

                case Spawner::PointStyle::Speed :
                    newVel.velX *= spawnVelocities.speed;
                    newVel.velY *= spawnVelocities.speed;
                    break;

                default: break;
                }

                Component<Position::name, Position> posComponent{newPos};
                Component<Velocity::name, Velocity> velComponent{newVel};
                manager->addComponent<Component<Position::name, Position>>(posComponent, newEntityID);
                manager->addComponent<Component<Velocity::name, Velocity>>(velComponent, newEntityID);

                if (spawner.addComponents.size() > 0) {
                    auto idx = i % spawner.addComponents.size();
                    std::vector<std::string>& cmpList = spawner.addComponents.at(idx);
                    for (std::string& cmp : cmpList) {
                        manager->addComponent(cmp, newEntityID);
                    }
                }

            }

            spawner.currentTime -= spawner.repeatRate;
            if (spawner.runCount > 0) {
                --spawner.runCount;
                if (spawner.runCount == 0) manager->removeComponent<Spawner>(entityID.first);
            }
        }
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}
