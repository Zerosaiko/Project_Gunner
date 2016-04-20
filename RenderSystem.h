#ifndef RENDERSYSTEM_H_INCLUDED
#define RENDERSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include "SDL_gpu.h"
#include <unordered_map>
#include <set>
#include <map>
#include "displace.h"
#include "renderable.h"
#include "window.h"
#include "Transform.h"
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

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<std::pair<EntityManager::ComponentHandle const *,EntityManager::ComponentHandle const *>> entities;

    std::deque<Component<Position::name, Position>>* positionPool;

    std::deque<Component<Sprite::name, Sprite>>* spritePool;

    std::deque<Component<Transform::name, Transform>>* transformPool;

    std::vector<int32_t> oldZOrderings;

    std::map<int32_t, std::set<uint32_t> > zOrderMap;

    std::unordered_map<std::string, SpriteSheet*> sprites;

    Window* window;

    //SDL_Texture* renderTarget;

    GPU_Image* targetImage;

};

#endif // RENDERSYSTEM_H_INCLUDED
