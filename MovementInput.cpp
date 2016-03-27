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
            auto player = entity->find("player");
            auto velocity = entity->find("velocity");
            auto delay = entity->find("fullDelay");
            auto pause = entity->find("pauseDelay");
            if ((delay == entity->end() || !delay->second.active )
                && (pause == entity->end() || !pause->second.active )
                && velocity != entity->end() && velocity->second.active && player != entity->end() && player->second.active) {
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
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !(entities[entityID->second].first->active && entities[entityID->second].second->active)) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID != entityIDs.end() ) {
        auto entity = manager->getEntity(id);
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ( (delay != entity->end() && delay->second.active) || (pause != entity->end() && pause->second.active) ) {
            freeIDXs.push_back(entityID->second);
            entityIDs.erase(entityID);
        }
    } else {
        addEntity(id);
    }

}

void MovementInputSystem::process(float dt) {

    int32_t keyNum = 0;
    uint8_t const * const keys = SDL_GetKeyboardState(&keyNum);

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Velocity& velocity = (*velocityPool)[entity.first->index].data;
        PlayerCmp& player = playerPool->operator[](entity.second->index).data;
        float spdMod = 0;
        if (player.playerNumber == 0) {
            if (!keys[inputMap["P1Focus"]]) spdMod = player.speed;
            else spdMod = player.focusSpeed;
            velocity.velX = (keys[inputMap["P1Move_Right"]] - keys[inputMap["P1Move_Left"]]) * spdMod;
            velocity.velY = (keys[inputMap["P1Move_Down"]] - keys[inputMap["P1Move_Up"]]) * spdMod;
        } else if (player.playerNumber == 1) {
            if (!keys[inputMap["P2Focus"]]) spdMod = player.speed;
            else spdMod = player.focusSpeed;
            velocity.velX = (keys[inputMap["P2Move_Right"]] - keys[inputMap["P2Move_Left"]]) * spdMod;
            velocity.velY = (keys[inputMap["P2Move_Down"]] - keys[inputMap["P2Move_Up"]]) * spdMod;
        }
    }

}

