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

            auto newIT = tfGraph.emplace(id, std::vector<uint32_t>(id)).first;
            Transform& tf = tfPool->operator[](position->second.index).data;
            oldLocals.emplace_back(tf.local);
            if (tf.hasParent) {
                newIT->second[0] = tf.parentTFEntity;
                tfGraph.emplace(tf.parentTFEntity, std::vector<uint32_t>(tf.parentTFEntity));
                tfGraph[tf.parentTFEntity].emplace_back(id);
            }

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

    auto it = tfGraph.equal_range(id).first;
    if (id != it->second[0]) {
        auto parentIT = tfGraph.equal_range(it->second[0]).first;
        std::vector<uint32_t>::size_type i = 0;
        for (; i < parentIT->second.size() && parentIT->second[i] != id; ++i);
        parentIT->second[i] = parentIT->second.back();
        parentIT->second.pop_back();
        for(auto child : it->second) {
            parentIT->second.push_back(child);
            tfGraph[child][0] = parentIT->first;
            if (child < entityIDXs.size() && hasEntity[child]) {
                Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                childTF.parentTFEntity = parentIT->first;

                TransformState& childLocal = childTF.local;

                GPU_MultiplyAndAssign(childLocal.matrix.data(), local.matrix.data());
                if (local.flipX) childLocal.flipX = !childLocal.flipX;
                if (local.flipY) childLocal.flipY = !childLocal.flipY;
                childLocal.angle += local.angle;
                childLocal.scaleX *= local.scaleX; childLocal.scaleY *= local.scaleY;
                childLocal.translateX += local.translateX; childLocal.translateY += local.translateY;
                childTF.dirty = true;

            }
        }
    } else {
        for (auto child : it->second) {
            tfGraph[child][0] = child;
            if (child < entityIDXs.size() && hasEntity[child]) {
                Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                childTF.parentTFEntity = child;

                TransformState& childLocal = childTF.local;

                GPU_MultiplyAndAssign(childLocal.matrix.data(), local.matrix.data());
                if (local.flipX) childLocal.flipX = !childLocal.flipX;
                if (local.flipY) childLocal.flipY = !childLocal.flipY;
                childLocal.angle += local.angle;
                childLocal.scaleX *= local.scaleX; childLocal.scaleY *= local.scaleY;
                childLocal.translateX += local.translateX; childLocal.translateY += local.translateY;
                childTF.dirty = true;

            }
        }
    }
    tfGraph.erase(it);
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

        auto it = tfGraph.equal_range(id).first;
        if (id != it->second[0]) {
            auto parentIT = tfGraph.equal_range(it->second[0]).first;
            std::vector<uint32_t>::size_type i = 0;
            for (; i < parentIT->second.size() && parentIT->second[i] != id; ++i);
            parentIT->second[i] = parentIT->second.back();
            parentIT->second.pop_back();
            for(auto child : it->second) {
                parentIT->second.push_back(child);
                tfGraph[child][0] = parentIT->first;
                if (child < entityIDXs.size() && hasEntity[child]) {
                    Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                    childTF.parentTFEntity = parentIT->first;

                    TransformState& childLocal = childTF.local;

                    GPU_MultiplyAndAssign(childLocal.matrix.data(), local.matrix.data());
                    if (local.flipX) childLocal.flipX = !childLocal.flipX;
                    if (local.flipY) childLocal.flipY = !childLocal.flipY;
                    childLocal.angle += local.angle;
                    childLocal.scaleX *= local.scaleX; childLocal.scaleY *= local.scaleY;
                    childLocal.translateX += local.translateX; childLocal.translateY += local.translateY;
                    childTF.dirty = true;

                }
            }
        } else {
            for (auto child : it->second) {
                tfGraph[child][0] = child;
                if (child < entityIDXs.size() && hasEntity[child]) {
                    Transform& childTF = tfPool->operator[](entities[entityIDXs[child]]->index).data;
                    childTF.parentTFEntity = child;

                    TransformState& childLocal = childTF.local;

                    GPU_MultiplyAndAssign(childLocal.matrix.data(), local.matrix.data());
                    if (local.flipX) childLocal.flipX = !childLocal.flipX;
                    if (local.flipY) childLocal.flipY = !childLocal.flipY;
                    childLocal.angle += local.angle;
                    childLocal.scaleX *= local.scaleX; childLocal.scaleY *= local.scaleY;
                    childLocal.translateX += local.translateX; childLocal.translateY += local.translateY;
                    childTF.dirty = true;

                }
            }
        }
        tfGraph.erase(it);
        Transform& tf = tfPool->operator[](entities[entityIDXs[id]]->index).data;
        auto newIT = tfGraph.emplace(id, std::vector<uint32_t>(id)).first;
        if (tf.hasParent) {
            newIT->second[0] = tf.parentTFEntity;
            tfGraph.emplace(tf.parentTFEntity, std::vector<uint32_t>(tf.parentTFEntity));
            tfGraph[tf.parentTFEntity].emplace_back(id);
        }
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
                GPU_MultiplyAndAssign(world.matrix.data(), parentWorld.matrix.data());
                if (parentWorld.flipX) world.flipX = !world.flipX;
                if (parentWorld.flipY) world.flipY = !world.flipY;
                world.angle += parentWorld.angle;
                world.scaleX *= parentWorld.scaleX, world.scaleY *= parentWorld.scaleY;
                world.translateX += parentWorld.translateX, world.translateY += parentWorld.translateY;
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
