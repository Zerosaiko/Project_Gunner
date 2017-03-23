#include "MovementSystem.h"
#include "SDL.h"
#include "SDL_gpu.h"

MovementSystem::MovementSystem(EntityManager* const manager, int32_t priority, TransformTree& tfGraph) : EntitySystem{manager, priority}, tfGraph(tfGraph) {
    tfPool = manager->getComponentPool<Component<Transform::name, Transform>>();
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
    auto entity = manager->getEntity(id);
    if (entity) {
        const auto &components = entity->components;
        auto position = components.find("transform");
        auto velocity = components.find("velocity");
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay == components.end() || !delay->second.active)
            && (pause == components.end() || !pause->second.active)
            && position != components.end() && velocity != components.end()
            && position->second.active && velocity->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&position->second, &velocity->second);
        }
    }
}

void MovementSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
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
        const auto &components = fullEntity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay != components.end() && delay->second.active) || (pause != components.end() && pause->second.active) ) {
            removeEntity(id);
        }
    }

}

void MovementSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    auto tfPool = this->tfPool.lock();
    auto velocityPool = this->velocityPool.lock();

    for(decltype(entities.size()) i = 0; i < entities.size(); ++i) {
        auto& entity = entities[i];
        Transform& position = (*tfPool)[entity.first->index].data;
        Velocity& velocity = (*velocityPool)[entity.second->index].data;
        if (velocity.velX || velocity.velY) {
            position.local.translate(velocity.velX * dt, velocity.velY * dt);
            tfGraph.setDirty(idxToID[i]);
        }
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}
