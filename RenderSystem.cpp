#include "RenderSystem.h"

RenderSystem::RenderSystem(EntityManager* const manager, int32_t priority, Window* window) : EntitySystem{manager, priority}, window(window) {
    targetImage = GPU_CreateImage(360, 480, GPU_FORMAT_RGBA);
    GPU_LoadTarget(targetImage);
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    spritePool = manager->getComponentPool<Component<Sprite::name, Sprite>>();
    orientationPool = manager->getComponentPool<Component<Orientation::name, Orientation>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
    oldZOrderings.reserve(1 << 16);
};

RenderSystem::~RenderSystem() {
    for( auto sprite = sprites.begin(); sprite != sprites.end(); delete sprite->second, sprite = sprites.erase(sprite));
    GPU_FreeTarget(targetImage->target);
    GPU_FreeImage(targetImage);
}

void RenderSystem::initialize() {}

void RenderSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto position = entity->find("position");
        auto render = entity->find("sprite");
        auto delay = entity->find("fullDelay");
        if ( (delay == entity->end() || !delay->second.active)
        && position != entity->end() && render != entity->end() && position->second.active && render->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&position->second, &render->second);

            Sprite& ren = (*spritePool)[render->second.index].data;

            ren.sheet = loadSprite(ren.spriteName);

            zOrderMap[ren.zOrder].insert(id);
            oldZOrderings.emplace_back(ren.zOrder);

        }
    }
}

void RenderSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    zOrderMap[oldZOrderings[entityIDXs[id]]].erase(id);
    oldZOrderings[entityIDXs[id]] = oldZOrderings.back();
    oldZOrderings.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void RenderSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity.first->active && entity.second->active)) {
        removeEntity(id);
    } else {
        const auto& fullEntity = manager->getEntity(id);
        auto delay = fullEntity->find("fullDelay");
        if ( (delay != fullEntity->end() && delay->second.active)) {
            removeEntity(id);
        } else if (entity.first->dirty) {

            Sprite& ren = (*spritePool)[entity.second->index].data;

            if (ren.zOrder != oldZOrderings[entityIDXs[id]]) {
                zOrderMap[oldZOrderings[entityIDXs[id]]].erase(id);
                zOrderMap[ren.zOrder].insert(id);
                oldZOrderings[entityIDXs[id]] = ren.zOrder;
            }

            ren.sheet = loadSprite(ren.spriteName);

        }
    }

}

void RenderSystem::process(float dt) {
}

void RenderSystem::render(float lerpT) {

    auto startT = SDL_GetPerformanceCounter();

    GPU_ClearRGB(targetImage->target, 64, 96, 64);

    for(const auto& layer : zOrderMap) {
        for (const auto& id : layer.second) {
            const auto& entity = entities[entityIDXs[id]];
            Position& position = (*positionPool)[entity.first->index].data;
            Sprite& render = (*spritePool)[entity.second->index].data;
            if (*render.sheet) {
                const auto& entity = manager->getEntity(id);
                const GPU_Rect& srcRect = *render.sheet->getSprite(render.spritePos);
                GPU_Rect dstRect = srcRect;

                dstRect.x = (int32_t)( position.pastPosX + (position.posX - position.pastPosX) * lerpT);
                dstRect.y = (int32_t)( position.pastPosY + (position.posY - position.pastPosY) * lerpT);

                auto orientationComponent = entity->find("orientation");
                if (orientationComponent == entity->end() || !orientationComponent->second.active) {
                    GPU_Blit(render.sheet->getImage(), render.sheet->getSprite(render.spritePos), targetImage->target, dstRect.x, dstRect.y);
                } else {
                    Orientation& orientation = orientationPool->operator[](orientationComponent->second.index).data;
                    GPU_PushMatrix();
                    GPU_Translate(dstRect.x, dstRect.y, 0);
                    GPU_Rotate(-orientation.angle, 0, 0, 1);
                    GPU_Scale(orientation.scaleX* (1 - 2 * orientation.flipX), orientation.scaleY * ( 1 - 2 * orientation.flipY), 1);
                    GPU_Blit(render.sheet->getImage(), render.sheet->getSprite(render.spritePos), targetImage->target, 0, 0);
                    GPU_PopMatrix();
                }
            }
        }
    }

    GPU_Rect targetDst{window->getWidth() / 2 - window->getHeight() / 3, 0, window->getHeight() * 2 / 3, window->getHeight()};
    GPU_BlitRectX(targetImage, nullptr, window->getTarget(), &targetDst, 0, 0, 0, GPU_FLIP_NONE);
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

        std::vector<std::string>::size_type pos = 0;
        int32_t cellWidth = buildFromString<int32_t>(finalData, pos);
        int32_t cellHeight = buildFromString<int32_t>(finalData, pos);
        int32_t cellSepWidth = buildFromString<int32_t>(finalData, pos);
        int32_t cellSepHeight = buildFromString<int32_t>(finalData, pos);
        metadata.close();
        std::string fileName = spriteName + ".png";
        GPU_Image* img = GPU_LoadImage(fileName.c_str());
        sprites[spriteName] = new SpriteSheet(img, cellWidth, cellHeight, cellSepWidth, cellSepHeight);
    }
    return sprites[spriteName];
}
