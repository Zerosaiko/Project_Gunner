#include "MovementInput.h"
#include "SDL.h"

MovementInputSystem::MovementInputSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    velocityPool = manager->getComponentPool<Component<Velocity::name, Velocity>>();
};

void MovementInputSystem::initialize() {}

void MovementInputSystem::addEntity(uint32_t id) {
    uint32_t* player = manager->tagManager.getIDByTag("player");
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity && player && *player == id) {
            auto velocity = entity->find("velocity");
            if (velocity != entity->end() && velocity->second.first) {
                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&velocity->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    entities[idx] = &velocity->second;
                }
            }
        }
    }
}

void MovementInputSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    }
}

void MovementInputSystem::refreshEntity(uint32_t id) {
    uint32_t* player = manager->tagManager.getIDByTag("player");
    auto entityID = entityIDs.find(id);
    if ((!player || *player != id) && entityID != entityIDs.end() && !(entities[entityID->second]->first)) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID == entityIDs.end() ) {
        addEntity(id);
    }

}

void MovementInputSystem::process(float dt) {

    int32_t keyNum = 0;
    uint8_t const * const keys = SDL_GetKeyboardState(&keyNum);
    auto speedPool = manager->getComponentPool<Component<PlayerCmp::speed, float>>();
    auto focusSpeedPool = manager->getComponentPool<Component<PlayerCmp::focusSpeed, float>>();

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        auto e = manager->getEntity(entityID.first);
        auto speed = e->find("speed");
        auto focusSpeed = e->find("focusSpeed");
        float spdMod = 0;
        if (speed != e->end() && !keys[inputMap["Focus"]]) spdMod = (*speedPool)[speed->second.second].data;
        else if (focusSpeed != e->end() && keys[inputMap["Focus"]]) spdMod = (*focusSpeedPool)[focusSpeed->second.second].data;
        Velocity& velocity = (*velocityPool)[entity->second].data;
        velocity.velX = (keys[inputMap["Move_Right"]] - keys[inputMap["Move_Left"]]) * spdMod;
        velocity.velY = (keys[inputMap["Move_Down"]] - keys[inputMap["Move_Up"]]) * spdMod;
    }

}

