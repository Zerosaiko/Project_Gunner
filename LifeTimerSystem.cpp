#include "LifeTimerSystem.h"

LifeTimerSystem::LifeTimerSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    lifeTimerPool = manager->getComponentPool<Component<lifeTimerName, float>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
};

void LifeTimerSystem::initialize() {}

void LifeTimerSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    auto entity = manager->getEntity(id);
    if (entity) {
        const auto &components = entity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        auto lifeT = components.find("lifeTimer");
        if ((delay == components.end() || !delay->second.active) && (pause == components.end() || !pause->second.active) &&
            (lifeT != components.end() && lifeT->second.active)) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&lifeT->second);

        }
    }
}

void LifeTimerSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void LifeTimerSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity->active)) {
        removeEntity(id);
    } else {
        auto fullEntity = manager->getEntity(id);
        const auto& components = fullEntity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ((delay != components.end() && delay->second.active) || (pause != components.end() && pause->second.active)) {
            removeEntity(id);
        }
    }

}

void LifeTimerSystem::process(float dt) {
    dt *= 1000.0f;

    auto lifeTimerPool = this->lifeTimerPool.lock();

    for(size_t i = 0; i < entities.size(); ++i) {
        const auto& entity = entities[i];
        float& lifeTime = (*lifeTimerPool)[entity->index].data;
        lifeTime -= dt;
        if (lifeTime <= 0.0f) manager->destroyEntity(idxToID[i]);
    }

}
