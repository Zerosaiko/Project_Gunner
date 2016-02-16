#include "RenderSystem.h"
#include "SDL_image.h"

RenderSystem::RenderSystem(EntityManager* const manager, int32_t priority, Window* window) : EntitySystem{manager, priority}, dirty(false), window(window) {
    renderTarget = SDL_CreateTexture(this->window->getRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 320, 480);
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    renderPool = manager->getComponentPool<Component<Renderable::name, Renderable>>();
};

RenderSystem::~RenderSystem() {
    for( auto sprite = sprites.begin(); sprite != sprites.end(); delete sprite->second, sprite = sprites.erase(sprite));
    SDL_DestroyTexture(renderTarget);
}

void RenderSystem::initialize() {}

void RenderSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto position = entity->find("position");
            auto render = entity->find("sprite");
            auto delay = entity->find("fullDelay");
            if ( (delay == entity->end() || !delay->second.first)
            && position != entity->end() && render != entity->end() && position->second.first && render->second.first) {
                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&position->second, &render->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    std::pair<EntityManager::component_pair const *, EntityManager::component_pair const *>
                        pos_ren_pair{&position->second, &render->second};
                    entities[idx] = pos_ren_pair;
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
    } else if (entityID != entityIDs.end() ) {
        auto entity = manager->getEntity(id);
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ( (delay != entity->end() && delay->second.first) || (pause != entity->end() && pause->second.first) ) {
            freeIDXs.push_back(entityID->second);
            entityIDs.erase(entityID);
        }
    } else {
        addEntity(id);
    }

}

void RenderSystem::process(float dt) {
}

void RenderSystem::render(float lerpT) {
    if (dirty) {
        dirty = false;
    }

    auto startT = SDL_GetPerformanceCounter();

    SDL_SetRenderTarget(window->getRenderer(), renderTarget);

    SDL_Rect targetSrc{0, 0, 320, 480};
    SDL_Rect targetDst{window->getWidth() / 2 - window->getHeight() / 3, 0, window->getHeight() * 2 / 3, window->getHeight()};

    SDL_SetRenderDrawColor(window->getRenderer(), 0, 255, 0, 255);

    SDL_RenderClear(window->getRenderer());

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Position& position = (*positionPool)[entity.first->second].data;
        Renderable& render = (*renderPool)[entity.second->second].data;
        if (*render.sheet) {
            const SDL_Rect& srcRect = *render.sheet->getSprite(render.spritePos);
            SDL_Rect dstRect = srcRect;
            dstRect.x = (int32_t)( position.pastPosX + (position.posX - position.pastPosX) * lerpT) - srcRect.w / 2;
            dstRect.y = (int32_t)( position.pastPosY + (position.posY - position.pastPosY) * lerpT) - srcRect.h / 2;
            /*
            std::cout << (position.pastPosX - srcRect.w / 2) << '\t'
            << (position.pastPosY - srcRect.h / 2) << '\n'
            << (position.posX - srcRect.w / 2) << '\t' << (position.posY - srcRect.h / 2) << '\n';
            std::cout << "Render pos: " << dstRect.x << '\t' << dstRect.y << '\n';
            */

            SDL_RenderCopy(window->getRenderer(), render.sheet->getTexture(), &srcRect, &dstRect);
        }
    }

    SDL_SetRenderTarget(window->getRenderer(), nullptr);

    SDL_RenderCopy(window->getRenderer(), renderTarget, &targetSrc, &targetDst);

    auto endT = SDL_GetPerformanceCounter();

    std::cout << "R-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

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
