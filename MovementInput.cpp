#include "MovementInput.h"
#include "SDL.h"
#include <iostream>

MovementInputSystem::MovementInputSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    velocityPool = manager->getComponentPool<Component<Velocity::name, Velocity>>();
    playerPool = manager->getComponentPool<Component<PlayerCmp::name, PlayerCmp>>();
};

void MovementInputSystem::initialize() {}

void MovementInputSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            const auto &components = entity->components;
            auto player = components.find("player");
            auto velocity = components.find("velocity");
            auto delay = components.find("fullDelay");
            auto pause = components.find("pauseDelay");
            if ((delay == components.end() || !delay->second.active )
                && (pause == components.end() || !pause->second.active )
                && velocity != components.end() && velocity->second.active && player != components.end() && player->second.active) {
                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&velocity->second, &player->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    entities[idx] = {&velocity->second, &player->second};
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
    return;
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !(entities[entityID->second].first->active && entities[entityID->second].second->active)) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID != entityIDs.end() ) {
        auto entity = manager->getEntity(id);
        const auto &components = entity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay != components.end() && delay->second.active) || (pause != components.end() && pause->second.active) ) {
            freeIDXs.push_back(entityID->second);
            entityIDs.erase(entityID);
        }
    } else {
        addEntity(id);
    }

}

void MovementInputSystem::process(float dt) {

    auto velocityPool = this->velocityPool.lock();
    auto playerPool = this->playerPool.lock();

    return;
    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Velocity& velocity = (*velocityPool)[entity.first->index].data;
        PlayerCmp& player = playerPool->operator[](entity.second->index).data;
        float spdMod = 0;
        if (player.playerNumber == 0) {
            auto focus = input.keyHeld(inputTypeMap["P1Focus"]);
            auto xMov = input.keyHeld(inputTypeMap["P1Move_Right"]) - input.keyHeld(inputTypeMap["P1Move_Left"]);
            auto yMov = input.keyHeld(inputTypeMap["P1Move_Down"]) - input.keyHeld(inputTypeMap["P1Move_Up"]);
            if (focus) spdMod = player.focusSpeed;
            else spdMod = player.speed;
            velocity.velX = xMov * spdMod;
            velocity.velY = yMov * spdMod;
        } else if (player.playerNumber == 1) {
            auto focus = input.keyHeld(inputTypeMap["P2Focus"]);
            auto xMov = input.keyHeld(inputTypeMap["P2Move_Right"]) - input.keyHeld(inputTypeMap["P2Move_Left"]);
            auto yMov = input.keyHeld(inputTypeMap["P2Move_Down"]) - input.keyHeld(inputTypeMap["P2Move_Up"]);
            if (focus) spdMod = player.focusSpeed;
            else spdMod = player.speed;
            velocity.velX = xMov * spdMod;
            velocity.velY = yMov * spdMod;
        }
    }

}

