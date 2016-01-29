#ifndef RENDERSYSTEM_H_INCLUDED
#define RENDERSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include <unordered_map>
#include "displace.h"
#include "renderable.h"
#include "window.h"
#include <fstream>

//  sort of hacky and incomplete system, should sort by z-order (texture doesn't matter because SDL has no concept of sprite batching sadly) if entities are added or removed
//  renders images from sprite sheets. No support for shapes currently
class RenderSystem : public EntitySystem {

public:
    RenderSystem(EntityManager* const manager, int32_t priority, Window* window);

    ~RenderSystem();

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

    void render(float lerpT);

    void onResize(int32_t width, int32_t height);

private:

    SpriteSheet* const loadSprite(std::string spriteName);

    std::unordered_map<uint32_t, std::vector<EntityManager::component_pair const *>::size_type> entityIDs;

    std::vector<std::vector<std::pair<EntityManager::component_pair const *,EntityManager::component_pair const *>>::size_type> freeIDXs;

    std::vector<std::pair<EntityManager::component_pair const *,EntityManager::component_pair const *>> entities;

    std::vector<Component<Position::name, Position>>* positionPool;

    std::vector<Component<Renderable::name, Renderable>>* renderPool;

    std::unordered_map<std::string, SpriteSheet*> sprites;

    bool dirty;

    Window* window;

    SDL_Texture* renderTarget;

};

#endif // RENDERSYSTEM_H_INCLUDED
