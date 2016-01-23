#include "MovementSystem.h"
#include "SDL.h"

MovementSystem::MovementSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    displacePool = manager->getComponentPool<Displace>();
};

void MovementSystem::initialize() {}

void MovementSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto displace = entity->find("displace");
            if (displace != entity->end()) {
                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.push_back(&displace->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    entities[idx] = &displace->second;
                }
            }
        }
    }
}

void MovementSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    }
}

void MovementSystem::refreshEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !entities[entityID->second]->first) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID == entityIDs.end() ) {
        addEntity(id);
    }

}

void MovementSystem::process(float dt) {

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Displace& displace = (*displacePool)[entity->second];
        displace.pastPosX = displace.posX;
        displace.pastPosY = displace.posY;
        displace.posX += dt * displace.velX;
        displace.posY += dt * displace.velY;
    }

}

