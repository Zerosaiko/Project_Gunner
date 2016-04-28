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
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        auto lifeT = entity->find("lifeTimer");
        if ((delay == entity->end() || !delay->second.active) && (pause == entity->end() || !pause->second.active) &&
            (lifeT != entity->end() && lifeT->second.active)) {

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
        const auto& fullEntity = manager->getEntity(id);
        auto delay = fullEntity->find("fullDelay");
        auto pause = fullEntity->find("pauseDelay");
        if ((delay != fullEntity->end() && delay->second.active) || (pause != fullEntity->end() && pause->second.active)) {
            removeEntity(id);
        }
    }

}

void LifeTimerSystem::process(float dt) {
    dt *= 1000.0f;

    for(size_t i = 0; i < entities.size(); ++i) {
        const auto& entity = entities[i];
        float& lifeTime = (*lifeTimerPool)[entity->index].data;
        lifeTime -= dt;
        if (lifeTime <= 0.0f) manager->destroyEntity(idxToID[i]);
    }

}
