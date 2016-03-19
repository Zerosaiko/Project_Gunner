#include "BoundsCorrection.h"
#include "SDL.h"

BoundsSystem::BoundsSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    boundsPool = manager->getComponentPool<Component<Bounds::name, Bounds>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
};

void BoundsSystem::initialize() {}

void BoundsSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);

    if (entity) {
        auto position = entity->find("position");
        auto bounds = entity->find("bounds");
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ((delay == entity->end() || !delay->second.active)
            && (pause == entity->end() || !pause->second.active)
            && position != entity->end() && bounds != entity->end() && position->second.active && bounds->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&position->second, &bounds->second);

            Position& pos = (*positionPool)[position->second.index].data;
            Bounds& b = (*boundsPool)[bounds->second.index].data;

            if (b.xBehavior == Bounds::Behavior::wrap || b.xBehavior == Bounds::Behavior::bounce) {
                if (pos.posX < b.minX) pos.posX = b.minX;
                else if (pos.posX > b.maxX) pos.posX = b.maxX;
                if (pos.pastPosX < b.minX) pos.pastPosX = b.minX;
                else if (pos.pastPosX > b.maxX) pos.pastPosX = b.maxX;
            }

            if (b.yBehavior == Bounds::Behavior::wrap || b.yBehavior == Bounds::Behavior::bounce) {
                if (pos.posY < b.minY) pos.posY = b.minY;
                else if (pos.posY > b.maxY) pos.posY = b.maxY;
                if (pos.pastPosY < b.minY) pos.pastPosY = b.minY;
                else if (pos.pastPosY > b.maxY) pos.pastPosY = b.maxY;
            }

        }
    }
}

void BoundsSystem::removeEntity(uint32_t id) {
    if (!hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void BoundsSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }

    const auto& entity = entities[entityIDXs[id]];
    if (!(entity.first->active && entity.second->active)) {
        removeEntity(id);
    }  else {
        auto fullEntity = manager->getEntity(id);
        auto delay = fullEntity->find("fullDelay");
        auto pause = fullEntity->find("pauseDelay");
        if ( (delay != fullEntity->end() && delay->second.active) || (pause != fullEntity->end() && pause->second.active) ) {
            removeEntity(id);
        } else if (entity.first->dirty || entity.second->dirty){

            Position& pos = (*positionPool)[entity.first->index].data;
            Bounds& b = (*boundsPool)[entity.second->index].data;

            if (b.xBehavior == Bounds::Behavior::wrap || b.xBehavior == Bounds::Behavior::bounce) {
                if (pos.posX < b.minX) pos.posX = b.minX;
                else if (pos.posX > b.maxX) pos.posX = b.maxX;
                if (pos.pastPosX < b.minX) pos.pastPosX = b.minX;
                else if (pos.pastPosX > b.maxX) pos.pastPosX = b.maxX;
            }

            if (b.yBehavior == Bounds::Behavior::wrap || b.yBehavior == Bounds::Behavior::bounce) {
                if (pos.posY < b.minY) pos.posY = b.minY;
                else if (pos.posY > b.maxY) pos.posY = b.maxY;
                if (pos.pastPosY < b.minY) pos.pastPosY = b.minY;
                else if (pos.pastPosY > b.maxY) pos.pastPosY = b.maxY;
            }

        }
    }

}

void BoundsSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    const auto& velocityPool = manager->getComponentPool<Component<Velocity::name, Velocity>>();

    for(size_t i = 0; i < entities.size(); ++i) {
        const auto& entity = entities[i];
        Position& position = (*positionPool)[entity.first->index].data;
        Bounds& bounds = (*boundsPool)[entity.second->index].data;

        if (bounds.xBehavior == Bounds::Behavior::block) {

            if (position.posX < bounds.minX) position.posX = bounds.minX;
            else if (position.posX > bounds.maxX) position.posX = bounds.maxX;

            if (position.pastPosX < bounds.minX) position.pastPosX = bounds.minX;
            else if (position.pastPosX > bounds.maxX) position.pastPosX = bounds.maxX;

        } else if (bounds.xBehavior == Bounds::Behavior::destroy
            && (position.posX < bounds.minX || position.posX > bounds.maxX) ) {

            manager->destroyEntity(idxToID[i]);

        } else if (bounds.xBehavior == Bounds::Behavior::wrap) {
            if (position.posX < bounds.minX) {
                position.pastPosX = bounds.maxX;
                position.posX = bounds.maxX - bounds.minX + position.posX;
            } else if (position.posX > bounds.maxX) {
                position.pastPosX = bounds.minX;
                position.posX = bounds.minX + position.posX - bounds.maxX;
            }
        } else if (bounds.xBehavior == Bounds::Behavior::bounce) {

            const auto& entity = manager->getEntity(idxToID[i]);

            auto velCmp = entity->find("velocity");

            if (velCmp != entity->end() && velCmp->second.active) {
                Velocity& velocity = velocityPool->operator[](velCmp->second.index).data;

                if (position.posX < bounds.minX) position.posX = bounds.minX,
                    velocity.velX = -velocity.velX, position.pastPosX = bounds.minX;
                else if (position.posX > bounds.maxX) position.posX = bounds.maxX,
                    velocity.velX = -velocity.velX, position.pastPosX = bounds.maxX;

                if (position.pastPosX < bounds.minX) position.pastPosX = bounds.minX;
                else if (position.pastPosX > bounds.maxX) position.pastPosX = bounds.maxX;
            }
        }

        if (bounds.yBehavior == Bounds::Behavior::block) {

            if (position.posY < bounds.minY) position.posY = bounds.minY;
            else if (position.posY > bounds.maxY) position.posY = bounds.maxY;

            if (position.pastPosY < bounds.minY) position.pastPosY = bounds.minY;
            else if (position.pastPosY > bounds.maxY) position.pastPosY = bounds.maxY;

        } else if (bounds.yBehavior == Bounds::Behavior::destroy
            && (position.posY < bounds.minY || position.posY > bounds.maxY) ) {

            manager->destroyEntity(idxToID[i]);

        } else if (bounds.yBehavior == Bounds::Behavior::wrap) {
            if (position.posY < bounds.minY) {
                position.pastPosY = bounds.maxY;
                position.posY = bounds.maxY - bounds.minY + position.posY;
            } else if (position.posY > bounds.maxY) {
                position.pastPosY = bounds.minY;
                position.posY = bounds.minY + position.posY - bounds.maxY;
            }
        } else if (bounds.yBehavior == Bounds::Behavior::bounce) {

            const auto& entity = manager->getEntity(idxToID[i]);

            auto velCmp = entity->find("velocity");

            if (velCmp != entity->end() && velCmp->second.active) {
                Velocity& velocity = velocityPool->operator[](velCmp->second.index).data;

                if (position.posY < bounds.minY) position.posY = bounds.minY,
                    velocity.velY = -velocity.velY, position.pastPosY = bounds.minY;
                else if (position.posY > bounds.maxY) position.posY = bounds.maxY,
                    velocity.velY = -velocity.velY, position.pastPosY = bounds.maxY;

                if (position.pastPosY < bounds.minY) position.pastPosY = bounds.minY;
                else if (position.pastPosY > bounds.maxY) position.pastPosY = bounds.maxY;
            }
        }
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "BOUNDS-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}




