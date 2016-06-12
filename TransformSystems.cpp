#include "TransformSystems.h"
#include "SDL_gpu.h"

TransformSyncSystem::TransformSyncSystem(EntityManager* const manager, int32_t priority, TransformTree& tfGraph) : EntitySystem{manager, priority}, tfGraph(tfGraph)  {
    tfPool = manager->getComponentPool<Component<Transform::name, Transform>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
    oldLocals.reserve(1 << 16);
};

void TransformSyncSystem::initialize() {}

void TransformSyncSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto position = entity->find("transform");
        if ( position != entity->end()
            && position->second.active ) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            unsetTFs.emplace(id);
            isUnset.emplace_back(true);
            entities.emplace_back(&position->second);

            auto newIT = tfGraph.transforms.emplace(id, TransformTree::Node(false, id)).first;
            Transform& tf = tfPool->operator[](position->second.index).data;
            oldLocals.emplace_back(tf.local);
            std::cout << tf.hasParent << '\n';
            if (tf.hasParent) {
                    std::cout << "Has parent: " << tf.parentTFEntity << "\n";
                newIT->second.parent = tf.parentTFEntity;
                auto parentIT = tfGraph.transforms.find(tf.parentTFEntity);
                if (parentIT == tfGraph.transforms.end()) {
                    auto parentIT = tfGraph.transforms.emplace(tf.parentTFEntity, TransformTree::Node(false, tf.parentTFEntity)).first;
                    parentIT->second.children.emplace_back(id);
                } else parentIT->second.children.emplace_back(id);
            }
            tfGraph.setDirty(id);

        }
    }
}



void TransformSyncSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    isUnset[entityIDXs[id]] = isUnset.back();
    isUnset.pop_back();
    Transform& tf = tfPool->operator[](entities[entityIDXs[id]]->index).data;
    TransformState& local = oldLocals[entityIDXs[id]];

    auto it = tfGraph.transforms.find(id);
    if (id != it->second.parent) {
        auto parentIT = tfGraph.transforms.find(it->second.parent);
        std::vector<uint32_t>::size_type i = 0;
        for (; i < parentIT->second.children.size() && parentIT->second.children[i] != id; ++i);
        parentIT->second.children[i] = parentIT->second.children.back();
        parentIT->second.children.pop_back();
        for(auto child : it->second.children) {
            parentIT->second.children.push_back(child);
            tfGraph.transforms[child].parent = parentIT->first;
            if (child < entityIDXs.size() && hasEntity[child]) {
                Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                childTF.parentTFEntity = parentIT->first;

                TransformState& childLocal = childTF.local;

                childLocal *= local;

                tfGraph.setDirty(child);

            }
        }
    } else {
        for (auto child : it->second.children) {
            tfGraph.transforms[child].parent = child;
            if (child < entityIDXs.size() && hasEntity[child]) {
                Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                childTF.parentTFEntity = child;

                TransformState& childLocal = childTF.local;

                childLocal *= local;

                tfGraph.setDirty(child);

            }
        }
    }
    tfGraph.transforms.erase(it);
    oldLocals[entityIDXs[id]] = oldLocals.back();
    oldLocals.pop_back();
    hasEntity[id] = false;
}

void TransformSyncSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity->active)) {
        removeEntity(id);
    } else if (entity->dirty) {
        isUnset[entityIDXs[id]] = true;
        TransformState& local = oldLocals[entityIDXs[id]];

        auto it = tfGraph.transforms.find(id);
        if (id != it->second.parent) {
            auto parentIT = tfGraph.transforms.find(it->second.parent);
            std::vector<uint32_t>::size_type i = 0;
            for (; i < parentIT->second.children.size() && parentIT->second.children[i] != id; ++i);
            parentIT->second.children[i] = parentIT->second.children.back();
            parentIT->second.children.pop_back();
            for(auto child : it->second.children) {
                parentIT->second.children.push_back(child);
                tfGraph.transforms[child].parent = parentIT->first;
                if (child < entityIDXs.size() && hasEntity[child]) {
                    Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                    childTF.parentTFEntity = parentIT->first;

                    TransformState& childLocal = childTF.local;

                    childLocal *= local;

                    tfGraph.setDirty(child);

                }
            }
        } else {
            for (auto child : it->second.children) {
                tfGraph.transforms[child].parent = child;
                if (child < entityIDXs.size() && hasEntity[child]) {
                    Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                    childTF.parentTFEntity = child;

                    TransformState& childLocal = childTF.local;

                    childLocal *= local;

                    tfGraph.setDirty(child);

                }
            }
        }
        tfGraph.transforms.erase(it);
        Transform& tf = tfPool->operator[](entities[entityIDXs[id]]->index).data;
        auto newIT = tfGraph.transforms.emplace(id, TransformTree::Node(false, id)).first;
        if (tf.hasParent) {
            newIT->second.parent = tf.parentTFEntity;

            tfGraph.transforms[tf.parentTFEntity].children.emplace_back(id);
        }
        tfGraph.setDirty(id);
    }

}

void TransformSyncSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    struct TFData {
        TFData(uint32_t id, Transform* tf) : id(id), tf(tf) {}
        uint32_t id;
        Transform* tf;
    };

    std::vector<TFData> tfStack;

    for (auto id : unsetTFs) {
        if (isUnset[id]) {
            tfStack.clear();
            Transform* tf = &tfPool->at(entities[entityIDXs[id]]->index).data;
            tfStack.emplace_back(id, tf);
            while(tf->hasParent && isUnset[entityIDXs[id]]) {
                tfStack.emplace_back(tf->parentTFEntity, tf);
                tf = &tfPool->at(entities[entityIDXs[tf->parentTFEntity]]->index).data;
            }
            tfStack.back().tf->worldPresent = tfStack.back().tf->local;
            isUnset[tfStack.back().id] = false;
            for (auto it = tfStack.rbegin() + 1; it != tfStack.rend(); ++it) {
                TFData& tfData = *it, &parentData = *(it - 1);
                TransformState& local = tfData.tf->local;
                TransformState& world = tfData.tf->worldPresent;
                TransformState& parentWorld = parentData.tf->worldPresent;
                world = local;
                world *= parentWorld;
                isUnset[tfData.id] = false;
            }
        }
    }
    unsetTFs.clear();

    for(std::vector<EntityManager::ComponentHandle const *>::size_type i = 0; i < entities.size(); ++i) {
        auto entity = entities[i];
        Transform& tf = tfPool->at(entity->index).data;
        tf.worldPast = tf.worldPresent;
        oldLocals[i] = tf.local;
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

TransformCalcSystem::TransformCalcSystem(EntityManager* const manager, int32_t priority, TransformTree& tfGraph) : EntitySystem{manager, priority}, tfGraph(tfGraph)  {
    tfPool = manager->getComponentPool<Component<Transform::name, Transform>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
};

void TransformCalcSystem::initialize() {}

void TransformCalcSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto position = entity->find("transform");
        if ( position != entity->end()
            && position->second.active ) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&position->second);

        }
    }
}



void TransformCalcSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void TransformCalcSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity->active)) {
        removeEntity(id);
    }

}

void TransformCalcSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    while(!tfGraph.dirtyList.empty()) {
        auto id = *tfGraph.dirtyList.begin();
        TransformTree::Node* tfNode = &tfGraph.transforms[id];
        while(tfNode->parent != id && tfNode->dirty) {
            id = tfNode->parent;
            tfNode = &tfGraph.transforms[tfNode->parent];
        }
        updateTransform(id, *tfNode);
        //std::cout << tfGraph.dirtyList.size() << '\n';
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

void TransformCalcSystem::updateTransform(uint32_t startID, TransformTree::Node& tfNode) {

    Transform& tf = tfPool->operator[](entities[entityIDXs[startID]]->index).data;
    tf.worldPresent = tf.local;
    if (startID != tfNode.parent) {

        Transform& parentTF = tfPool->operator[](entities[entityIDXs[tfNode.parent]]->index).data;
        TransformState& local = tf.local;
        TransformState& world = tf.worldPresent;
        TransformState& parentWorld = parentTF.worldPresent;
        world = local;

        world *= parentWorld;

    }
    //std::cout << "S: " << tfNode.children.size() << '\n';
    updateTransformChildren(tfNode.children);
    tfGraph.clearDirty(startID);

}

void TransformCalcSystem::updateTransformChildren(std::vector<uint32_t> children) {
    for(auto child : children) {
        auto& tfNode = tfGraph.transforms[child];
        children.insert(children.end(), tfNode.children.begin(), tfNode.children.end());
        Transform& tf = tfPool->operator[](entities[entityIDXs[child]]->index).data;
        Transform& parentTF = tfPool->operator[](entities[entityIDXs[tfNode.parent]]->index).data;
        TransformState& local = tf.local;
        TransformState& world = tf.worldPresent;
        TransformState& parentWorld = parentTF.worldPresent;
        world = local;

        world *= parentWorld;

        tfGraph.clearDirty(child);
    }

}

