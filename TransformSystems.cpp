#include "TransformSystems.h"
#include "SDL_gpu.h"

TransformSyncSystem::TransformSyncSystem(EntityManager* const manager, int32_t priority, TransformTree& tfGraph) : EntitySystem{manager, priority}, tfGraph(tfGraph)  {
    tfPool = manager->getComponentPool<Component<Transform::name, Transform>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
    oldLocals.reserve(1 << 16);
    oldHeights.reserve(1 << 16);
};

void TransformSyncSystem::initialize() {}

void TransformSyncSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    auto entity = manager->getEntity(id);
    if (entity) {
        const auto &components = entity->components;
        auto position = components.find("transform");
        if ( position != components.end()
            && position->second.active ) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            isUnset.emplace_back(1);
            entities.emplace_back(&position->second);

            auto currentIT = tfGraph.transforms.find(id);
            if (currentIT == tfGraph.transforms.end())
                currentIT = tfGraph.transforms.emplace(id, TransformTree::Node(id)).first;
            auto savedIT = currentIT;

            Transform* currentTF = &tfPool.lock()->operator[](position->second.index).data;
            currentTF->worldPast = currentTF->worldPresent = currentTF->local;
            oldLocals.emplace_back(currentTF->local);

            auto currentID = id;

            if (currentTF->hasParent) {
                uint32_t parentID = currentTF->parentTFEntity;

                currentIT->second.parent = parentID;
                tfGraph.transforms[parentID].children.emplace_back(currentID);

            }

            {
                std::vector<TransformTree::Node*> tfStack;
                uint32_t height = 0;
                uint32_t currentID = id;
                while (currentTF->hasParent) {

                    tfStack.emplace_back(&currentIT->second);
                    //std::cout << "Put " << currentID << " on stack\n";
                    uint32_t parentID = currentTF->parentTFEntity;
                    auto parentIT = tfGraph.transforms.find(parentID);

                    bool alreadyIn = true;
                    if (parentIT == tfGraph.transforms.end()) {
                        parentIT = tfGraph.transforms.emplace(parentID, TransformTree::Node(parentID)).first;
                        alreadyIn = false;
                    }
                    const auto parentEntity = manager->getEntity(parentID);
                    if (!parentEntity) {
                        std::cerr << "Parent does not exist: " << parentID << '\n';
                        return;
                    }
                    const auto &components = parentEntity->components;
                    auto transformComp = components.find("transform");
                    if (transformComp == components.end()) {
                        std::cerr << "Parent does not have transform: " << parentID << '\n';
                        return;
                    }
                    Transform& parentTF = tfPool.lock()->operator[](transformComp->second.index).data;

                    currentTF = &parentTF;
                    currentID = parentID;

                    if (alreadyIn && parentIT->second.height != 0) {
                        //std::cout << parentID << " already in\n";
                        height = parentIT->second.height;
                        break;
                    }

                }


                for (auto node = tfStack.rbegin(); node != tfStack.rend(); ++node) {
                    (*node)->height = ++height;
                    //std::cout << "Parent: " << (*node)->parent << '\t' << "Height: " << (*node)->height << '\n';
                }
                //std::cout << "Added entity " << id << " - height = " << tfGraph.transforms[id].height << '\n';

            }
            oldHeights.emplace_back(savedIT->second.height);
            unsetTFs[savedIT->second.height].emplace(id);

            tfGraph.setDirty(id);
        }
    }
}

void TransformSyncSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    auto idx = entityIDXs[id];
    entities[idx] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = idx;
    idxToID[idx] = idxToID.back();
    idxToID.pop_back();
    isUnset[idx] = isUnset.back();
    isUnset.pop_back();
    auto tfPool = this->tfPool.lock();
    TransformState& local = oldLocals[entityIDXs[id]];
    std::vector<uint32_t> children = tfGraph.transforms[id].children;
    uint32_t parent = tfGraph.transforms[id].parent;
    uint32_t childCount = children.size();
    if (parent != id) {
        auto& parentNode = tfGraph.transforms[parent];
        parentNode.children.erase(std::remove(parentNode.children.begin(), parentNode.children.end(), id));
        for(std::size_t i = 0; i < children.size(); i++) {
            uint32_t child = children[i];
            auto& childNode = tfGraph.transforms[child];
            childNode.height--;
            children.insert(children.end(), childNode.children.begin(), childNode.children.end());
            if (i < childCount) {
                Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                childNode.parent = parent;
                childTF.parentTFEntity = parent;
                TransformState& childLocal = childTF.local;
                childLocal *= local;
                tfGraph.setDirty(child);
            }
        }
    }
    else {
        for(std::size_t i = 0; i < children.size(); i++) {
            uint32_t child = children[i];
            auto& childNode = tfGraph.transforms[child];
            childNode.height--;
            children.insert(children.end(), childNode.children.begin(), childNode.children.end());
            if (i < childCount) {
                Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                childNode.parent = child;
                childTF.hasParent = false;
                childTF.parentTFEntity = 0;
                TransformState& childLocal = childTF.local;
                childLocal *= local;
                tfGraph.setDirty(child);
            }
        }
    }
    tfGraph.transforms.erase(id);
    oldLocals[idx] = oldLocals.back();
    oldLocals.pop_back();
    unsetTFs[oldHeights[idx]].erase(id);
    oldHeights[idx] = oldHeights.back();
    oldHeights.pop_back();
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
        auto tfPool = this->tfPool.lock();
        TransformState& local = oldLocals[entityIDXs[id]];
        std::vector<uint32_t> children = tfGraph.transforms[id].children;
        uint32_t parent = tfGraph.transforms[id].parent;
        uint32_t childCount = children.size();
        if (parent != id) {
            auto& parentNode = tfGraph.transforms[parent];
            parentNode.children.erase(std::remove(parentNode.children.begin(), parentNode.children.end(), id));
            for(std::size_t i = 0; i < children.size(); i++) {
                uint32_t child = children[i];
                auto& childNode = tfGraph.transforms[child];
                childNode.height--;
                children.insert(children.end(), childNode.children.begin(), childNode.children.end());
                if (i < childCount) {
                    Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                    childNode.parent = parent;
                    childTF.parentTFEntity = parent;
                    TransformState& childLocal = childTF.local;
                    childLocal *= local;
                    tfGraph.setDirty(child);
                }
            }
        }
        else {
            for(std::size_t i = 0; i < children.size(); i++) {
                uint32_t child = children[i];
                auto& childNode = tfGraph.transforms[child];
                childNode.height--;
                children.insert(children.end(), childNode.children.begin(), childNode.children.end());
                if (i < childCount) {
                    Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                    childNode.parent = child;
                    childTF.hasParent = false;
                    childTF.parentTFEntity = 0;
                    TransformState& childLocal = childTF.local;
                    childLocal *= local;
                    tfGraph.setDirty(child);
                }
            }
        }
        tfGraph.transforms.erase(id);
        auto currentIT = tfGraph.transforms.emplace(id, TransformTree::Node(id)).first;
        auto savedIT = currentIT;

        Transform* currentTF = &tfPool->operator[](entity->index).data;
        currentTF->worldPast = currentTF->worldPresent = currentTF->local;
        oldLocals.emplace_back(currentTF->local);

        auto currentID = id;

        if (currentTF->hasParent) {
            uint32_t parentID = currentTF->parentTFEntity;

            currentIT->second.parent = parentID;
            tfGraph.transforms[parentID].children.emplace_back(currentID);

        }

        {
            std::vector<TransformTree::Node*> tfStack;
            uint32_t height = 0;
            uint32_t currentID = id;
            while (currentTF->hasParent) {

                tfStack.emplace_back(&currentIT->second);
                //std::cout << "Put " << currentID << " on stack\n";
                uint32_t parentID = currentTF->parentTFEntity;
                auto parentIT = tfGraph.transforms.find(parentID);

                bool alreadyIn = true;
                if (parentIT == tfGraph.transforms.end()) {
                    parentIT = tfGraph.transforms.emplace(parentID, TransformTree::Node(parentID)).first;
                    alreadyIn = false;
                }
                const auto parentEntity = manager->getEntity(parentID);
                if (!parentEntity) {
                    std::cerr << "Parent does not exist: " << parentID << '\n';
                    return;
                }
                const auto &components = parentEntity->components;
                auto transformComp = components.find("transform");
                if (transformComp == components.end()) {
                    std::cerr << "Parent does not have transform: " << parentID << '\n';
                    return;
                }
                Transform& parentTF = tfPool->operator[](transformComp->second.index).data;

                currentTF = &parentTF;
                currentID = parentID;

                if (alreadyIn && parentIT->second.height != 0) {
                    //std::cout << parentID << " already in\n";
                    height = parentIT->second.height;
                    break;
                }

            }


            for (auto node = tfStack.rbegin(); node != tfStack.rend(); ++node) {
                (*node)->height = ++height;
                //std::cout << "Parent: " << (*node)->parent << '\t' << "Height: " << (*node)->height << '\n';
            }
            //std::cout << "Added entity " << id << " - height = " << tfGraph.transforms[id].height << '\n';

        }
        oldHeights.emplace_back(savedIT->second.height);
        unsetTFs[savedIT->second.height].emplace(id);

        tfGraph.setDirty(id);
    }

}

void TransformSyncSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    auto tfPool = this->tfPool.lock();

    struct TFData {
        TFData(uint32_t id, Transform* tf) : id(id), tf(tf) {}
        uint32_t id;
        Transform* tf;
    };

    std::vector<TFData> tfStack;

    for (auto &&heightLevel : unsetTFs) {
        for (auto id : heightLevel.second) {
            //std::cout << "for id " << id << ":\n\t";
            auto tfCmpHdl = entities[entityIDXs[id]];
            auto &tf = tfPool->operator[](tfCmpHdl->index).data;
            if (heightLevel.first != 0) {
                auto parentTFHdl = entities[entityIDXs[tf.parentTFEntity]];
                auto &parentTF = tfPool->operator[](parentTFHdl->index).data;
                tf.worldPresent = tf.local;
                tf.worldPresent *= parentTF.worldPresent;
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

    //std::cout << "TFS-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

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
    auto entity = manager->getEntity(id);
    if (entity) {
        const auto &components = entity->components;
        auto position = components.find("transform");
        if ( position != components.end()
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
        uint32_t id = *tfGraph.dirtyList.begin();
        TransformTree::Node* tfNode = &tfGraph.transforms[id];
        uint32_t parentID = tfNode->parent;
        while(parentID != id) {
            id = tfNode->parent;
            tfNode = &tfGraph.transforms[id];
            parentID = tfNode->parent;
        }

        tfGraph.clearDirty(id);
        updateTransform(id, *tfNode);
        //std::cout << tfGraph.dirtyList.size() << '\n';
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "TFC-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

void TransformCalcSystem::updateTransform(uint32_t startID, TransformTree::Node& tfNode) {

    auto tfPool = this->tfPool.lock();
    Transform& tf = tfPool->at(entities.at(entityIDXs.at(startID))->index).data;

    TransformState& local = tf.local;
    TransformState& world = tf.worldPresent;
    world = local;
    if (tfNode.parent != startID) {
        Transform& parentTF = tfPool->operator[](entities[entityIDXs[tfNode.parent]]->index).data;
        TransformState& parentWorld = parentTF.worldPresent;

        world *= parentWorld;
    }


    //std::cout << "S: " << tfNode.children.size() << '\n';

    for(auto child : tfNode.children) {
        updateTransform(child, tfGraph.transforms[child]);
    }

}

void TransformCalcSystem::updateTransformChildren(std::vector<uint32_t> children) {
    for(auto child : children) {
        updateTransform(child, tfGraph.transforms[child]);
    }

}

