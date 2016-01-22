#ifndef RENDERSYSTEM_H_INCLUDED
#define RENDERSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include <unordered_map>
#include "displace.h"
#include "renderable.h"

class RenderSystem : public EntitySystem {

public:
    RenderSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

    void render(float lerpT);

private:

    std::unordered_map<uint32_t, std::vector<EntityManager::component_pair const *>::size_type> entityIDs;

    std::vector<std::vector<std::pair<EntityManager::component_pair const *,EntityManager::component_pair const *>>::size_type> freeIDXs;

    std::vector<std::pair<EntityManager::component_pair const *,EntityManager::component_pair const *>> entities;

    std::vector<Displace>* displacePool;

    std::vector<Component<Renderable::name, Renderable>>* renderPool;

    std::unordered_map<std::string, SpriteSheet> sprites;

    SpriteSheet* const loadSprite(std::string spriteName);

    bool dirty;

};

#endif // RENDERSYSTEM_H_INCLUDED
