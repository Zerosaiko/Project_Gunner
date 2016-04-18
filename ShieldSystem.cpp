#include "ShieldSystem.h"

ShieldSystem::ShieldSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority},
    onShieldHit(std::function<bool(Message&)>([this](Message& message){
            return shieldHit(message); } ) ) {
    shieldPool = manager->getComponentPool<Component<Shield::name, Shield>>();
    manager->registerWithMessage(Message::Type::PlayerHit, onShieldHit, 500);
}

void ShieldSystem::initialize() {

}

void ShieldSystem::addEntity(uint32_t id) {
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto shieldCmp = entity->find("shield");
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ( (delay == entity->end() || !delay->second.active) && (pause == entity->end() || !pause->second.active)
            && shieldCmp != entity->end() && shieldCmp->second.active) {

            entities.emplace(id, &shieldCmp->second);

        }
    }

}

void ShieldSystem::removeEntity(uint32_t id) {
    entities.erase(id);
}

void ShieldSystem::refreshEntity(uint32_t id) {
    auto entity = entities.find(id);
    if (entity != entities.end() && !entity->second->active) {
        removeEntity(id);
    } else if (entity != entities.end()) {
        const auto& fullEntity = manager->getEntity(id);
        auto delay = fullEntity->find("fullDelay");
        auto pause = fullEntity->find("pauseDelay");
        if ( (delay != fullEntity->end() && delay->second.active) || (pause != fullEntity->end() && pause->second.active) ) {
            removeEntity(id);
        }
    } else {
        addEntity(id);
    }
}

void ShieldSystem::process(float dt) {

    float adjustedDT = dt * 1000.0f;

    for (auto& entity : entities) {

        Shield& shieldData = shieldPool->operator[](entity.second->index).data;
        shieldData.timeLimit -= adjustedDT;
        if (shieldData.timeLimit <= 0) {
            manager->removeComponent<Component<Shield::name, Shield>>(entity.first);
        }

    }

}

bool ShieldSystem::shieldHit(Message& message) {//
    PlayerHit& data = (PlayerHit&)message;
    auto entity = entities.find(data.col2ID);
    if (entity != entities.end() && entity->second->active) {
        std::cout << "Shield hit\tHitter: " << data.col1ID << "\tShield: " << data.col2ID << '\n';

        return false;
    }
    return true;
}
