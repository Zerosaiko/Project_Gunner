#include "HealthSystem.h"

HealthSystem::HealthSystem(EntityManager* const manager, int32_t priority, sol::state &luaState) : EntitySystem{manager, priority} {
    healthPool = manager->getComponentPool<Component<Health::name, Health>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);

    luaState["healthSystem"] = this;
    luaState.new_usertype<HealthSystem>("HealthSystem",
        "changeHealth", &HealthSystem::changeHealth

    );
}

void HealthSystem::initialize() {}

void HealthSystem::addEntity(uint32_t id) {
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
        auto health = components.find("health");
        if ((delay == components.end() || !delay->second.active) && (pause == components.end() || !pause->second.active) &&
            (health != components.end() && health->second.active)) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&health->second);

        }
    }
}

void HealthSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void HealthSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity->active)) {
        removeEntity(id);
    } else {
        auto fullEntity = manager->getEntity(id);
        const auto &components = fullEntity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay != components.end() && delay->second.active) || (pause != components.end() && pause->second.active) ) {
            removeEntity(id);
        }
    }
}

void HealthSystem::process(float dt) {
    dt *= 1000.0f;

    auto healthPool = this->healthPool.lock();

    for(size_t i = 0; i < entities.size(); ++i) {
        const auto entity = entities[i];
        auto& entHealth = healthPool->operator[](entity->index).data;
        if (entHealth.current <= 0) {
            manager->destroyEntity(idxToID[i]);
        }
    }

}

bool HealthSystem::changeHealth(uint32_t id, int16_t amt) {
    if (id >= hasEntity.size() || !hasEntity[id]) return false;
    using std::min;
    auto idx = entityIDXs[id];
    auto& entHealth = healthPool.lock()->operator[](entities[idx]->index).data;
    if (entHealth.current <= 0) return false;
    entHealth.current = min(entHealth.max, entHealth.current + amt);
    /*if (entHealth.current <= 0) {
        manager->destroyEntity(idxToID[i]);
    }*/
    return true;

}
