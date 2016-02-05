#include "CollisionSystem.h"
#include "SDL.h"
#include <iostream>

CollisionSystem::CollisionSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    colliderPool = manager->getComponentPool<Component<Collider::name, Collider>>();
};

void CollisionSystem::initialize() {}

void CollisionSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto collider = entity->find("collider");
            if (collider != entity->end() && collider->second.first) {
                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&collider->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    entities[idx] = &collider->second;
                }
            }
        }
    }
}

void CollisionSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    }
}

void CollisionSystem::refreshEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !(entities[entityID->second]->first)) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID == entityIDs.end() ) {
        addEntity(id);
    }

}

void CollisionSystem::process(float dt) {

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Collider& collider = (*colliderPool)[entity->second].data;
    }

}

