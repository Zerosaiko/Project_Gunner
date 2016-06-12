#ifndef TRANSFORMSYSTEMS_H_INCLUDED
#define TRANSFORMSYSTEMS_H_INCLUDED

#include "EntitySystem.h"
#include "Transform.h"
#include <deque>
#include <vector>
#include <unordered_set>

class TransformSyncSystem : public EntitySystem {

public:
    TransformSyncSystem(EntityManager* const manager, int32_t priority, TransformTree& tfGraph);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<uint8_t> isUnset;

    std::vector<EntityManager::ComponentHandle const *> entities;

    std::deque<Component<Transform::name, Transform>>* tfPool;

    std::unordered_set<uint32_t> unsetTFs;

    std::vector<TransformState> oldLocals;

    TransformTree& tfGraph;

};

class TransformCalcSystem : EntitySystem {

public:
    TransformCalcSystem(EntityManager* const manager, int32_t priority, TransformTree& tfGraph);

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

    std::deque<Component<Transform::name, Transform>>* tfPool;

    TransformTree& tfGraph;

    void updateTransform(uint32_t startID, TransformTree::Node& tfNode);

    void updateTransformChildren(std::vector<uint32_t> children);

};

#endif // TRANSFORMSYSTEMS_H_INCLUDED
