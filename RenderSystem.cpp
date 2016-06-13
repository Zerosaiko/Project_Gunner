#include "RenderSystem.h"

RenderSystem::RenderSystem(EntityManager* const manager, int32_t priority, Window* window) : EntitySystem{manager, priority}, window(window) {
    targetImage = GPU_CreateImage(360, 480, GPU_FORMAT_RGBA);
    GPU_LoadTarget(targetImage);
    spritePool = manager->getComponentPool<Component<Sprite::name, Sprite>>();
    transformPool = manager->getComponentPool<Component<Transform::name, Transform>>();
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
        auto transform = entity->find("transform");
        auto render = entity->find("sprite");
        auto delay = entity->find("fullDelay");
        if ( (delay == entity->end() || !delay->second.active)
        && transform != entity->end() && render != entity->end() && transform->second.active && render->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&transform->second, &render->second);

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
        } else if (entity.second->dirty) {

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
            Transform& transform = (*transformPool)[entity.first->index].data;
            Sprite& render = (*spritePool)[entity.second->index].data;

            SpriteInfo& sprInfo = render.sheet->getSprite(render.spritePos);
            GPU_Rect& srcRect = sprInfo.getRect();
            const std::array<float, 4> texCoords = sprInfo.getTexCoords();
            TransformState interp;
            TransformState& past = transform.worldPast;
            TransformState& present = transform.worldPresent;

            interp.setOrigin(past.originX + lerpT * (present.originX - past.originX),
                            past.originY + lerpT * (present.originY - past.originY));
            interp.translate(past.translateX + lerpT * (present.translateX - past.translateX),
                                past.translateY + lerpT * (present.translateY - past.translateY));
            interp.rotate(past.angle + lerpT * (present.angle - past.angle));
            interp.scale(past.scaleX + lerpT * (present.scaleX - past.scaleX),
                            past.scaleY + lerpT * (present.scaleY - past.scaleY));
            interp.setFlipX(present.flipX);
            interp.setFlipY(present.flipY);

            float halfWidth = srcRect.w / 2.0f;
            float halfHeight = srcRect.h / 2.0f;
            float  tl[] = {-halfWidth, -halfHeight, 1},
            tr[] = {halfWidth, -halfHeight, 1},
            bl[] = {-halfWidth, halfHeight, 1},
            br[] = {halfWidth, halfHeight, 1};

            GPU_VectorApplyMatrix(tl, interp.matrix.data());
            GPU_VectorApplyMatrix(tr, interp.matrix.data());
            GPU_VectorApplyMatrix(bl, interp.matrix.data());
            GPU_VectorApplyMatrix(br, interp.matrix.data());

            float verts[] = {
                            tl[0], tl[1], texCoords[0], texCoords[3], 1, 1, 1, 1,
                            tr[0], tr[1], texCoords[2], texCoords[3], 1, 1, 1, 1,
                            br[0], br[1], texCoords[2], texCoords[1], 1, 1, 1, 1,
                            bl[0], bl[1], texCoords[0], texCoords[1], 1, 1, 1, 1,
                            };
            unsigned short ind[] = {0, 1, 2, 2, 3, 0};

            GPU_TriangleBatch(render.sheet->getImage(), targetImage->target, 4, verts, 6, ind, GPU_BATCH_XY_ST_RGBA);
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
