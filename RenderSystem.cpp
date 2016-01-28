#include "RenderSystem.h"
#include "SDL_image.h"

RenderSystem::RenderSystem(EntityManager* const manager, int32_t priority, Window* window) : EntitySystem{manager, priority}, dirty(false), window(window) {
    renderTarget = SDL_CreateTexture(this->window->getRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 320, 480);
    displacePool = manager->getComponentPool<Displace>();
    renderPool = manager->getComponentPool<Component<Renderable::name, Renderable>>();
};

RenderSystem::~RenderSystem() {
    std::vector<SpriteSheet*> delSprites;
    for( auto& sprite: sprites) {
        delSprites.push_back(sprite.second);
    }
    for (auto& sprite : delSprites) {
        delete sprite;
        sprite = nullptr;
    }
    SDL_DestroyTexture(renderTarget);
}

void RenderSystem::initialize() {}

void RenderSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto displace = entity->find("displace");
            auto render = entity->find("sprite");
            if (displace != entity->end() && render != entity->end() && displace->second.first && render->second.first) {
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
                dirty = true;

                Renderable& ren = (*renderPool)[render->second.second].data;

                ren.sheet = loadSprite(ren.spriteName);
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
    if (entityID != entityIDs.end() && !(entities[entityID->second].first->first && entities[entityID->second].second->first)) {
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

    auto startT = SDL_GetPerformanceCounter();

    SDL_SetRenderTarget(window->getRenderer(), renderTarget);

    SDL_Rect targetSrc{0, 0, 320, 480};
    SDL_Rect targetDst{160, 0, 320, 480};

    SDL_SetRenderDrawColor(window->getRenderer(), 0, 255, 0, 255);

    SDL_RenderClear(window->getRenderer());

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Displace& displace = (*displacePool)[entity.first->second];
        Renderable& render = (*renderPool)[entity.second->second].data;
        if (*render.sheet) {
            const SDL_Rect& srcRect = *render.sheet->getSprite(render.spritePos);
            SDL_Rect dstRect = srcRect;
            dstRect.x = (int)(displace.pastPosX + displace.velX * lerpT);
            dstRect.y = (int)(displace.pastPosY + displace.velY * lerpT);


            SDL_RenderCopy(window->getRenderer(), render.sheet->getTexture(), &srcRect, &dstRect);
        }
    }

    SDL_SetRenderTarget(window->getRenderer(), nullptr);

    SDL_RenderCopy(window->getRenderer(), renderTarget, &targetSrc, &targetDst);

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "R-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

SpriteSheet* const RenderSystem::loadSprite(std::string spriteName) {
    auto sprite = sprites.find(spriteName);
    if (sprite == sprites.end()) {
        std::ifstream metadata{spriteName + ".txt"};
        std::string cWidth;
        std::string cHeight;
        std::string cSepW;
        std::string cSepH;
        metadata >> cWidth;
        metadata >> cHeight;
        metadata >> cSepW;
        metadata >> cSepH;

        std::vector<std::string> finalData{{cWidth, cHeight, cSepW, cSepH}};

        int32_t cellWidth = buildFromString<int32_t>(finalData, 0);
        int32_t cellHeight = buildFromString<int32_t>(finalData, 1);
        int32_t cellSepWidth = buildFromString<int32_t>(finalData, 2);
        int32_t cellSepHeight = buildFromString<int32_t>(finalData, 3);
        metadata.close();
        spriteName += ".png";
        SDL_Surface* surf = IMG_Load(spriteName.c_str());
        SpriteSheet* sheet = new SpriteSheet(surf, cellWidth, cellHeight, cellSepWidth, cellSepHeight, window);
        sprites[spriteName] = sheet;
    }
    return sprites[spriteName];
}
