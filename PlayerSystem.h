#ifndef PLAYERSYSTEM_H_INCLUDED
#define PLAYERSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include "playerComponents.h"
#include "Message.h"
#include <random>
#include "sol.hpp"

class PlayerSystem : public EntitySystem {

public:
    PlayerSystem(EntityManager* const manager, int32_t priority, sol::state& state);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

    bool playerHit(uint32_t id1, uint32_t id2);

private:
    std::unordered_map<uint32_t, EntityManager::ComponentHandle const *> entities;

    std::weak_ptr<std::deque<Component<PlayerCmp::name, PlayerCmp>>> playerPool;

    std::unordered_map<uint32_t, PlayerHit> playerCollisions;

    std::default_random_engine randEngine;

    std::uniform_real_distribution<float> floatDist;

    std::function<bool(Message& message)> playerHitFunc;

    sol::state& luaState;

};

#endif // PLAYERSYSTEM_H_INCLUDED
