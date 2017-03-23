#include "RenderSystem.h"
#include <string>

RenderSystem::RenderSystem(EntityManager* const manager, int32_t priority, Window* window) : EntitySystem{manager, priority},
    window(window), imageFilterMode(GPU_FILTER_LINEAR_MIPMAP),
    defaultShaderProg( std::vector<shader::Shader>(
        {
            shader::Shader(GPU_VERTEX_SHADER,
                std::vector<shader::ShaderInput>(
                    {
                        shader::ShaderInput("gpu_Vertex", shader::Vec3),
                        shader::ShaderInput("gpu_TexCoord", shader::Vec2),
                        shader::ShaderInput("gpu_Color", shader::Vec4)
                    }
                ),
                std::vector<shader::ShaderInput>(
                    {
                        shader::ShaderInput("gpu_ModelViewProjectionMatrix", shader::Mat4),
                        shader::ShaderInput("transform", shader::Mat4),
                        shader::ShaderInput("tex", shader::Sampler2D)
                    }
                ),
                std::vector<shader::ShaderInput>(
                    {
                        shader::ShaderInput("color", shader::Vec4),
                        shader::ShaderInput("texCoord", shader::Vec2)
                    }
                ),
                std::vector<shader::ShaderFunction>(
                    {
                        shader::ShaderFunction(shader::Void,
                        std::vector<shader::Parameter>()
                        , "getColorAndTexCoord", "color = gpu_Color;\ntexCoord = gpu_TexCoord;"),
                        shader::ShaderFunction(shader::Void,
                        std::vector<shader::Parameter>()
                        , "transformVerts", "gl_Position = gpu_ModelViewProjectionMatrix * transform * vec4(gpu_Vertex, 1.0);")
                    }
                )
            ),
            shader::Shader(GPU_FRAGMENT_SHADER,
                std::vector<shader::ShaderInput>(),
                std::vector<shader::ShaderInput>(
                    {
                        shader::ShaderInput("gpu_ModelViewProjectionMatrix", shader::Mat4),
                        shader::ShaderInput("transform", shader::Mat4),
                        shader::ShaderInput("tex", shader::Sampler2D)
                    }
                ),
                std::vector<shader::ShaderInput>(
                    {
                        shader::ShaderInput("color", shader::Vec4),
                        shader::ShaderInput("texCoord", shader::Vec2)
                    }
                ),
                std::vector<shader::ShaderFunction>(
                    {
                        shader::ShaderFunction(shader::Void,
                        std::vector<shader::Parameter>()
                        , "testFragCall", "gl_FragColor = texture2D(tex, texCoord) * color;")
                    }
                )
            )
        }
    )
    ),
    currentShaderProg(&defaultShaderProg) {
    targetImage = GPU_CreateImage(300, 400, GPU_FORMAT_RGBA);
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
    //for( auto sprite = sprites.begin(); sprite != sprites.end(); delete sprite->second, sprite = sprites.erase(sprite));
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
    auto entity = manager->getEntity(id);
    if (entity) {
        const auto &components = entity->components;
        auto transform = components.find("transform");
        auto render = components.find("sprite");
        auto delay = components.find("fullDelay");
        if ( (delay == components.end() || !delay->second.active)
        && transform != components.end() && render != components.end() && transform->second.active && render->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&transform->second, &render->second);

            Sprite& ren = (*spritePool.lock())[render->second.index].data;

            ren.sheet = loadSprite(ren.spriteName).lock();

            zOrderMap[ren.zOrder].insert(id);
            oldZOrderings.emplace_back(ren.zOrder);

        }
    }
}

void RenderSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    Sprite& ren = (*spritePool.lock())[entities[entityIDXs[id]].second->index].data;
    ren.sheet = nullptr;
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
        auto fullEntity = manager->getEntity(id);
        const auto &components = fullEntity->components;
        auto delay = components.find("fullDelay");
        if ( (delay != components.end() && delay->second.active)) {
            removeEntity(id);
        } else if (entity.second->dirty) {

            Sprite& ren = (*spritePool.lock())[entity.second->index].data;

            if (ren.zOrder != oldZOrderings[entityIDXs[id]]) {
                zOrderMap[oldZOrderings[entityIDXs[id]]].erase(id);
                zOrderMap[ren.zOrder].insert(id);
                oldZOrderings[entityIDXs[id]] = ren.zOrder;
            }

            ren.sheet = loadSprite(ren.spriteName).lock();

        }
    }

}

void RenderSystem::process(float dt) {

}

void RenderSystem::render(float lerpT) {

    auto startT = SDL_GetPerformanceCounter();

    GPU_ClearRGB(targetImage->target, 64, 96, 64);

    auto transformPool = this->transformPool.lock();
    auto spritePool = this->spritePool.lock();
    //GPU_SetUniformi(GPU_GetUniformLocation(defaultShaderProg.getObj(), "tex"), 0);

    for(const auto& layer : zOrderMap) {
        for (const auto& id : layer.second) {
            const auto& entity = entities[entityIDXs[id]];
            Transform& transform = (*transformPool)[entity.first->index].data;
            Sprite& render = (*spritePool)[entity.second->index].data;

            SpriteInfo& sprInfo = render.sheet->getSprite(render.spritePos);
            GPU_Rect srcRect = sprInfo.getRect();
            TransformState interp;
            TransformState& past = transform.worldPast;
            TransformState& present = transform.worldPresent;

            float xPos, yPos, xPosPast, yPosPast;

            std::tie(xPos, yPos) = present.getPos();

            std::tie(xPosPast, yPosPast) = past.getPos();

            interp.setOrigin(past.originX + lerpT * (present.originX - past.originX),
                            past.originY + lerpT * (present.originY - past.originY));
            interp.translate(xPosPast + lerpT * (xPos - xPosPast),
                                yPosPast + lerpT * (yPos - yPosPast));
            interp.rotate(past.angle + lerpT * (present.angle - past.angle));
            interp.scale(past.scaleX + lerpT * (present.scaleX - past.scaleX),
                            past.scaleY + lerpT * (present.scaleY - past.scaleY));
            interp.setFlipX(present.flipX);
            interp.setFlipY(present.flipY);

            //GPU_SetUniformMatrixfv(GPU_GetUniformLocation(defaultShaderProg.getObj(), "transform"), 1, 4, 4, 0, interp.matrix.data() );

            block = currentShaderProg->loadBlock();
            currentShaderProg->activate(block);
            currentShaderProg->setUniform("transform", interp.matrix);
            /*
            const std::array<float, 4> texCoords = sprInfo.getTexCoords();
            auto presPos = present.getPos();
            auto pastPos = past.getPos();
            auto interpPos = interp.getPos();

            float halfWidth = srcRect.w / 2.0f;
            float halfHeight = srcRect.h / 2.0f;
            float  tl[] = {-halfWidth, -halfHeight},
            tr[] = {halfWidth, -halfHeight},
            bl[] = {-halfWidth, halfHeight},
            br[] = {halfWidth, halfHeight};

            float verts[] = {
                tl[0], tl[1], texCoords[0], texCoords[3], 1, 1, 1, 1,
                tr[0], tr[1], texCoords[2], texCoords[3], 1, 1, 1, 1,
                br[0], br[1], texCoords[2], texCoords[1], 1, 1, 1, 1,
                bl[0], bl[1], texCoords[0], texCoords[1], 1, 1, 1, 1,
            };
            unsigned short ind[] = {0, 1, 2, 2, 3, 0};
            //GPU_TriangleBatch(render.sheet->getImage(), targetImage->target, 4, verts, 6, ind, GPU_BATCH_XY_ST_RGBA);*/
            currentShaderProg->sendInputs();
            //GPU_SetRGBA(render.sheet->getImage(), 255, 255, 255, 60);
            GPU_Blit(render.sheet->getImage(), &srcRect, targetImage->target, 0.0f, 0.0f);
        }
    }

    GPU_ActivateShaderProgram(0, nullptr);

    GPU_Rect targetDst{window->getWidth() / 2 - window->getHeight() / 3, 0, window->getHeight() * 2 / 3, window->getHeight()};
    GPU_BlitRectX(targetImage, nullptr, window->getTarget(), &targetDst, 0, 0, 0, GPU_FLIP_NONE);
    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "R-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';
}

std::weak_ptr<SpriteSheet> const RenderSystem::loadSprite(std::string spriteName) {
    auto sprite = sprites.find(spriteName);
    if (sprite == sprites.end()) {
        std::ifstream metadata{spriteName + ".txt"};
        std::string cWidth;
        std::string cHeight;
        std::string cSepW;
        std::string cSepH;
        int32_t cellWidth = 0;
        int32_t cellHeight = 0;
        int32_t cellSepWidth = 0;
        int32_t cellSepHeight = 0;
        metadata >> cellWidth >> cellHeight >> cellSepWidth >> cellSepHeight;
        metadata.close();
        std::string fileName = spriteName + ".png";
        GPU_Image* img = GPU_LoadImage(fileName.c_str());
        GPU_SetImageFilter(img, imageFilterMode);
        GPU_SetBlendMode(img, GPU_BLEND_NORMAL);
        sprite = sprites.emplace(spriteName,
            std::shared_ptr<SpriteSheet>(new SpriteSheet(img, cellWidth, cellHeight, cellSepWidth, cellSepHeight))).first;
    }
    return sprite->second;
}
