#ifndef BOUNDSCORRECTION_H_INCLUDED
#define BOUNDSCORRECTION_H_INCLUDED

#include "EntitySystem.h"
#include <map>
#include "Transform.h"
#include "boundsComponent.h"

class BoundsSystem : public EntitySystem {

public:
    BoundsSystem(EntityManager* const manager, int32_t priority, TransformTree& tfGraph);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::map<uint32_t, std::vector<uint32_t>> idxToID;

    std::map<uint32_t, std::vector<std::pair<EntityManager::ComponentHandle const *,EntityManager::ComponentHandle const *>> > entities;

    std::vector<uint32_t> nodeHeights;

    std::weak_ptr<std::deque<Component<Transform::name, Transform>>> tfPool;

    std::weak_ptr<std::deque<Component<Bounds::name, Bounds>>> boundsPool;

    TransformTree& tfGraph;

    void updateTransforms(Transform& tf, Bounds& bounds, uint32_t id, float xPres, float yPres, float xPast, float yPast);

    void precheckTransforms();

};

#endif // BOUNDSCORRECTION_H_INCLUDED
