#ifndef ANIMATIONSYSTEM_H_INCLUDED
#define ANIMATIONSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include "renderable.h"
#include "animation.h"

class AnimationSystem : public EntitySystem {

public:
    AnimationSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint32_t> backIDs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<std::pair<EntityManager::ComponentHandle const *,EntityManager::ComponentHandle const *>> entities;

    std::weak_ptr<std::deque<Component<Animation::name, Animation>>> animationPool;

    std::weak_ptr<std::deque<Component<Sprite::name, Sprite>>> spritePool;

};

#endif // ANIMATIONSYSTEM_H_INCLUDED
