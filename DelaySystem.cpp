#include "DelaySystem.h"
#include "SDL.h"
#include <iostream>

DelaySystem::DelaySystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    delayPool = manager->getComponentPool<Component<delayComponent::fullDelay, float>>();
};

void DelaySystem::initialize() {}

void DelaySystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto delay = entity->find("fullDelay");
            if (delay != entity->end() && delay->second.first) {
                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&delay->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    entities[idx] = &delay->second;
                }
            }
        }
    }
}

void DelaySystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    }
}

void DelaySystem::refreshEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !(entities[entityID->second]->first)) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID == entityIDs.end() ) {
        addEntity(id);
    }

}

void DelaySystem::process(float dt) {
    dt *= 1000.0f;

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        float& delay = (*delayPool)[entity->second].data;
        delay -= dt;
        if (delay <= 0.0f) manager->removeComponent<Component<delayComponent::fullDelay, float>>(entityID.first);
    }

}

