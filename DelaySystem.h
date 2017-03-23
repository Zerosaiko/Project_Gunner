#ifndef DELAYSYSTEM_H_INCLUDED
#define DELAYSYSTEM_H_INCLUDED

#include "delayComponent.h"
#include "EntitySystem.h"
#include <memory>

class DelaySystem : public EntitySystem {

public:
    DelaySystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<EntityManager::ComponentHandle const *> entities;

    std::weak_ptr<std::deque<Component<delayComponent::fullDelay, float>>> delayPool;

};

class PauseSystem : public EntitySystem {

public:
    PauseSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<EntityManager::ComponentHandle const *> entities;

    std::weak_ptr<std::deque<Component<delayComponent::pauseDelay, float>>> pausePool;

};

#endif // DELAYSYSTEM_H_INCLUDED
