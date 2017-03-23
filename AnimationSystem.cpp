#include "AnimationSystem.h"
#include "displace.h"

AnimationSystem::AnimationSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    animationPool = manager->getComponentPool<Component<Animation::name, Animation>>();
    spritePool = manager->getComponentPool<Component<Sprite::name, Sprite>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
};

void AnimationSystem::initialize() {}

void AnimationSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    auto entity = manager->getEntity(id);
    if (entity) {
        auto &components = entity->components;
        auto animation = components.find("animation");
        auto sprite = components.find("sprite");
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay == components.end() || !delay->second.active)
            && (pause == components.end() || !pause->second.active)
            && animation != components.end() && sprite != components.end()
            && animation->second.active && sprite->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&animation->second, &sprite->second);

            Animation& anim = (*animationPool.lock())[animation->second.index].data;
            Sprite& spr = (*spritePool.lock())[sprite->second.index].data;
            spr.spritePos = anim.frames[anim.currentIdx];
        }
    }
}

void AnimationSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void AnimationSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity.first->active && entity.second->active)) {
        removeEntity(id);
    } else {
        auto fullEntity = manager->getEntity(id);
        const auto& components = fullEntity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay != components.end() && delay->second.active) || (pause != components.end() && pause->second.active) ) {
            removeEntity(id);
        } else  if (entity.first->dirty || entity.second->dirty){
            Animation& animation = (*animationPool.lock())[entity.first->index].data;
            Sprite& sprite = (*spritePool.lock())[entity.second->index].data;
            sprite.spritePos = animation.frames[animation.currentIdx];
        }
    }

}

void AnimationSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    dt *= 1000.0f;
    auto animationPool = this->animationPool.lock();
    auto spritePool = this->spritePool.lock();

    for(auto& entity : entities) {
        Animation& animation = (*animationPool)[entity.first->index].data;
        Sprite& sprite = (*spritePool)[entity.second->index].data;

        animation.currentTime += dt;
        while (animation.currentTime >= animation.frameLengths[animation.currentIdx] && animation.frameLengths[animation.currentIdx] >= 0) {
            animation.currentTime -= animation.frameLengths[animation.currentIdx];
            ++animation.currentIdx;
            if (animation.currentIdx >= animation.frames.size()) {
                switch (animation.endBehavior) {
                    case Animation::EndBehavior::None :
                        animation.currentIdx = animation.frames.size() - 1;
                        break;
                    case Animation::EndBehavior::Loop :
                        animation.currentIdx = animation.loopIdx;
                        break;

                }
            }
            sprite.spritePos = animation.frames[animation.currentIdx];
        }
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}
