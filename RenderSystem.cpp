#include "RenderSystem.h"

RenderSystem::RenderSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority}, dirty(false) {
    displacePool = manager->getComponentPool<Displace>();
};

void RenderSystem::initialize() {}

void RenderSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto displace = entity->find("displace");
            auto render = entity->find("sprite");
            if (displace != entity->end() && render != entity->end()) {
                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&displace->second, &render->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    std::pair<EntityManager::component_pair const *, EntityManager::component_pair const *>
                        disp_ren_pair{&displace->second, &render->second};
                    entities[idx] = disp_ren_pair;
                }
                (*renderPool)[render->second.second].data.sheet = nullptr;
                dirty = true;
            }
        }
    }
}

void RenderSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
        dirty = true;
    }
}

void RenderSystem::refreshEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && entities[entityID->second].first->first && entities[entityID->second].second->first) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID == entityIDs.end() ) {
        addEntity(id);
    }

}

void RenderSystem::process(float dt) {
}

void RenderSystem::render(float lerpT) {
    if (dirty) {

    }

    for(std::pair<EntityManager::component_pair const *, EntityManager::component_pair const * >& entity : entities) {
        Displace& displace = (*displacePool)[entity.first->second];
        Renderable& render = (*renderPool)[entity.second->second].data;



    }

}

SpriteSheet* const RenderSystem::loadSprite(std::string spriteName) {

}
