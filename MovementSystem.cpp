#include "MovementSystem.h"
#include "SDL.h"

MovementSystem::MovementSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    velocityPool = manager->getComponentPool<Component<Velocity::name, Velocity>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
};

void MovementSystem::initialize() {}

void MovementSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto position = entity->find("position");
        auto velocity = entity->find("velocity");
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ( (delay == entity->end() || !delay->second.active)
            && (pause == entity->end() || !pause->second.active)
            && position != entity->end() && velocity != entity->end()
            && position->second.active && velocity->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&position->second, &velocity->second);
        }
    }
}

void MovementSystem::removeEntity(uint32_t id) {
    if (!hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void MovementSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity.first->active && entity.second->active)) {
        removeEntity(id);
    } else {
        const auto& fullEntity = manager->getEntity(id);
        auto delay = fullEntity->find("fullDelay");
        auto pause = fullEntity->find("pauseDelay");
        if ( (delay != fullEntity->end() && delay->second.active) || (pause != fullEntity->end() && pause->second.active) ) {
            removeEntity(id);
        }
    }

}

void MovementSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    for(auto& entity : entities) {
        Position& position = (*positionPool)[entity.first->index].data;
        Velocity& velocity = (*velocityPool)[entity.second->index].data;
        position.pastPosX = position.posX;
        position.pastPosY = position.posY;
        position.posX += dt * velocity.velX;
        position.posY += dt * velocity.velY;
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

PositionSyncSystem::PositionSyncSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
};

void PositionSyncSystem::initialize() {}

void PositionSyncSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto position = entity->find("position");
        if ( position != entity->end()
            && position->second.active ) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&position->second);
        }
    }
}

void PositionSyncSystem::removeEntity(uint32_t id) {
    if (!hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void PositionSyncSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity->active)) {
        removeEntity(id);
    }

}

void PositionSyncSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    for(size_t i = 0; i < entities.size(); ++i) {
        auto& entity = entities[i];
        Position& position = positionPool->at(entity->index).data;
        position.pastPosX = position.posX;
        position.pastPosY = position.posY;
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

