#include "BoundsCorrection.h"
#include "SDL.h"
#include "displace.h"
#include "SDL_gpu.h"
#include <cmath>

float wrap(float value, float wrapMax) {

    return value - std::floor(value / wrapMax) * wrapMax;
}

float wrap(float value, float wrapMin, float wrapMax) {
    return wrap(value - wrapMin, wrapMax - wrapMin) + wrapMin;
}

BoundsSystem::BoundsSystem(EntityManager* const manager, int32_t priority, TransformTree& tfGraph) : EntitySystem{manager, priority}, tfGraph(tfGraph) {
    tfPool = manager->getComponentPool<Component<Transform::name, Transform>>();
    boundsPool = manager->getComponentPool<Component<Bounds::name, Bounds>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    nodeHeights.reserve(1 << 16);
};

void BoundsSystem::initialize() {}

void BoundsSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
        nodeHeights.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);

    if (entity) {
        auto transform = entity->components.find("transform");
        auto bounds = entity->components.find("bounds");
        auto delay = entity->components.find("fullDelay");
        auto pause = entity->components.find("pauseDelay");
        if ((delay == entity->components.end() || !delay->second.active)
            && (pause == entity->components.end() || !pause->second.active)
            && transform != entity->components.end() && bounds != entity->components.end() && transform->second.active && bounds->second.active) {

            auto height = tfGraph.transforms[id].height;

            entityIDXs[id] = entities[height].size();
            hasEntity[id] = true;
            nodeHeights[id] = height;
            idxToID[height].reserve(1 << 8);
            entities[height].reserve(1 << 8);
            idxToID[height].emplace_back(id);
            entities[height].emplace_back(&transform->second, &bounds->second);

            Transform& tf = tfPool.lock()->operator[](transform->second.index).data;
            Bounds& b = (*boundsPool.lock())[bounds->second.index].data;

            /*
            float xDiff = 0.f, yDiff = 0.f;
            float xToBorder = 0.f, yToBorder = 0.f;
            float xPos = 0, yPos = 0;
            tf.worldPresent.getPos(xPos, yPos);

            if (b.xBehavior == Bounds::Behavior::wrap || b.xBehavior == Bounds::Behavior::bounce) {
                if (xPos < b.minX) xDiff = xToBorder = (b.minX - xPos);
                else if (xPos > b.maxX) xDiff = xToBorder = (b.maxX - xPos);
            }

            if (b.yBehavior == Bounds::Behavior::wrap || b.yBehavior == Bounds::Behavior::bounce) {
                if (yPos < b.minY) yDiff = yToBorder = (b.minY - yPos);
                else if (yPos > b.maxY) yDiff = yToBorder = (b.maxY - yPos);
            }

            if (xDiff != 0.0f || yDiff != 0.0f) {
                updateTransforms(tf, id, xDiff, yDiff, xToBorder, yToBorder);
            }
            */
        }
    }
}

void BoundsSystem::removeEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) return;
    auto height = nodeHeights[id];
    entities[height][entityIDXs[id]] = entities[height].back();
    entities[height].pop_back();
    entityIDXs[idxToID[height].back()] = entityIDXs[id];
    idxToID[height][entityIDXs[id]] = idxToID[height].back();
    idxToID[height].pop_back();
    hasEntity[id] = false;
}

void BoundsSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    auto oldHeight = nodeHeights[id];
    const auto& entity = entities[oldHeight][entityIDXs[id]];
    if (!(entity.first->active && entity.second->active)) {
        removeEntity(id);
    }  else {
        auto fullEntity = manager->getEntity(id);
        const auto& components = fullEntity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay != components.end() && delay->second.active) || (pause != components.end() && pause->second.active) ) {
            removeEntity(id);
        } else if (entity.first->dirty || entity.second->dirty){

            Transform& tf = tfPool.lock()->operator[](entity.first->index).data;
            Bounds& b = (*boundsPool.lock())[entity.second->index].data;
            auto height = tfGraph.transforms[id].height;
            if (height != oldHeight) {
                auto entity = entities[oldHeight][entityIDXs[id]];
                entities[oldHeight][entityIDXs[id]] = entities[oldHeight].back();
                entities[oldHeight].pop_back();
                entityIDXs[idxToID[oldHeight].back()] = entityIDXs[id];
                idxToID[oldHeight][entityIDXs[id]] = idxToID[oldHeight].back();
                idxToID[oldHeight].pop_back();

                entityIDXs[id] = entities[height].size();
                idxToID[height].emplace_back(id);
                entities[height].emplace_back(entity);
                nodeHeights[id] = height;
            }

            /*
            if (b.xBehavior == Bounds::Behavior::wrap || b.xBehavior == Bounds::Behavior::bounce) {
                if (pos.posX < b.minX) pos.posX = b.minX;
                else if (pos.posX > b.maxX) pos.posX = b.maxX;
                if (pos.pastPosX < b.minX) pos.pastPosX = b.minX;
                else if (pos.pastPosX > b.maxX) pos.pastPosX = b.maxX;
            }

            if (b.yBehavior == Bounds::Behavior::wrap || b.yBehavior == Bounds::Behavior::bounce) {
                if (pos.posY < b.minY) pos.posY = b.minY;
                else if (pos.posY > b.maxY) pos.posY = b.maxY;
                if (pos.pastPosY < b.minY) pos.pastPosY = b.minY;
                else if (pos.pastPosY > b.maxY) pos.pastPosY = b.maxY;
            }
            */

        }
    }

}

void BoundsSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    //std::cout << "start\n";
    auto velocityPool = manager->getComponentPool<Component<Velocity::name, Velocity>>().lock();
    auto tfPool = this->tfPool.lock();
    auto boundsPool = this->boundsPool.lock();
    for (auto& heightLevel : entities) {
        //std::cout << "Height: " << heightLevel.first << '\n';
        for(size_t i = 0; i < heightLevel.second.size(); ++i) {
            const auto& entity = heightLevel.second[i];
            Transform& transform = (*tfPool)[entity.first->index].data;
            Bounds& bounds = (*boundsPool)[entity.second->index].data;

            auto id = idxToID[heightLevel.first][i];
            //std::cout << "ID: " << id << '\n';
            float xPos = 0, yPos = 0;
            float xPast = 0, yPast = 0;
            float maxX = bounds.maxX - bounds.minX, maxY = bounds.maxY - bounds.minY;
            std::tie(xPos, yPos) = transform.worldPresent.getPos();
            std::tie(xPast, yPast) = transform.worldPast.getPos();
            xPos -= bounds.minX;
            yPos -= bounds.minY;

            float xPres = xPos, yPres = yPos;
            xPast -= bounds.minX;
            yPast -= bounds.minY;

            //std::cout << id << ": " << xPos << ' ' << yPos << '\n' << '\t';
            //std::cout << xPast << ' ' << yPast << '\n';

            switch (bounds.limitType) {
            case Bounds::LimitType::none :
                break;

            case Bounds::LimitType::time :
                if (bounds.limitType == Bounds::LimitType::time) {
                    bounds.timeLimit -= dt;
                }
                break;

            case Bounds::LimitType::boundsLimit :
                if (bounds.boundsLimit.x > 0 && (xPos < 0 || xPos > maxX)) {
                    --bounds.boundsLimit.x;
                }

                if (bounds.boundsLimit.y > 0 && (yPos < 0 || yPos > maxY)) {
                    --bounds.boundsLimit.y;
                }
                break;

            }

            if (bounds.xBehavior == Bounds::Behavior::block) {

                if (xPos < 0)  {
                    xPres = 0;
                } else if (xPos > maxX) {
                    xPres = maxX;
                }

            } else if (bounds.xBehavior == Bounds::Behavior::destroy
                && (xPos < 0 || xPos > maxX) ) {

                manager->destroyEntity(id);

            } else if (bounds.xBehavior == Bounds::Behavior::wrap) {
                using namespace std;
                if (xPos < 0 || xPos > maxX) {
                    float xPastDiff = xPres - xPast;
                    xPres = wrap(xPos, maxX);
                    xPast = xPres - xPastDiff;

                }
            } else if (bounds.xBehavior == Bounds::Behavior::bounce) {

                const auto& entity = manager->getEntity(id);

                auto velCmp = entity->components.find("velocity");

                if (velCmp != entity->components.end() && velCmp->second.active) {
                    Velocity& velocity = velocityPool->operator[](velCmp->second.index).data;

                    if (xPos < 0) {
                        xPres = xPast = 0;
                        velocity.velX = -velocity.velX;
                    } else if (xPos > maxX) {
                        xPres = xPast = maxX;
                        velocity.velX = -velocity.velX;
                    }
                }
            }

            if (bounds.yBehavior == Bounds::Behavior::block) {

                if (yPos < 0)  {
                    yPres = 0;
                } else if (yPos > maxY) {
                    yPres = maxY;
                }

            } else if (bounds.yBehavior == Bounds::Behavior::destroy
                && (yPos < 0 || yPos > maxY) ) {

                manager->destroyEntity(id);

            } else if (bounds.yBehavior == Bounds::Behavior::wrap) {
                using namespace std;
                if (yPos < 0 || yPos > maxY) {
                    float yPastDiff = yPres - yPast;
                    yPres = wrap(yPos, maxY);
                    yPast = yPres - yPastDiff;

                }
            } else if (bounds.yBehavior == Bounds::Behavior::bounce) {

                const auto& entity = manager->getEntity(id);

                auto velCmp = entity->components.find("velocity");

                if (velCmp != entity->components.end() && velCmp->second.active) {
                    Velocity& velocity = velocityPool->operator[](velCmp->second.index).data;

                    if (yPos < 0) {
                        yPres = yPast = 0;
                        velocity.velY = -velocity.velY;
                    } else if (yPos > maxY) {
                        yPres = yPast = maxY;
                        velocity.velY = -velocity.velY;
                    }
                }
            }

            if (xPres != xPos || yPres != yPos) {
                updateTransforms(transform, bounds, id, xPres + bounds.minX,
                    yPres + bounds.minY, xPast + bounds.minX, yPast + bounds.minY);
                std::tie(xPos, yPos) = transform.worldPresent.getPos();
                std::tie(xPast, yPast) = transform.worldPast.getPos();
                //std::cout << '\t' << xPos << ' ' << yPos << "\n\t";
                //std::cout << xPast << ' ' << yPast << '\n';

            }


            switch (bounds.limitType) {
            case Bounds::LimitType::none :
                break;

            case Bounds::LimitType::time :
                if (bounds.timeLimit <= 0.0f) {
                    switch (bounds.postLimit) {
                    case Bounds::PostLimitBehavior::none : break;

                    case Bounds::PostLimitBehavior::change :
                        manager->addComponent("bounds", bounds.changeBounds, id);
                        break;

                    case Bounds::PostLimitBehavior::destroy : manager->destroyEntity(id); break;

                    }
                }
                break;

            case Bounds::LimitType::boundsLimit :

                if (bounds.boundsLimit.x == 0) {
                    switch (bounds.postLimit) {
                    case Bounds::PostLimitBehavior::none : break;

                    case Bounds::PostLimitBehavior::change :
                        manager->addComponent("bounds", bounds.changeBounds, id);
                        break;

                    case Bounds::PostLimitBehavior::destroy : bounds.xBehavior = bounds.yBehavior = Bounds::Behavior::destroy; break;

                    }
                }

                if (bounds.boundsLimit.y == 0) {
                    switch (bounds.postLimit) {
                    case Bounds::PostLimitBehavior::none : break;

                    case Bounds::PostLimitBehavior::change :
                        manager->addComponent("bounds", bounds.changeBounds, id);
                        break;

                    case Bounds::PostLimitBehavior::destroy : bounds.xBehavior = bounds.yBehavior = Bounds::Behavior::destroy; break;

                    }
                }
                break;

            }

        }
    }

    //std::cout << "end\n";
    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "BOUNDS-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

inline void BoundsSystem::updateTransforms(Transform& transform, Bounds& bounds, uint32_t id,
    float xPres, float yPres, float xPast, float yPast) {

    float xPosPres = 0, yPosPres = 0, xPosPast = 0, yPosPast = 0;
    std::tie(xPosPres, yPosPres) = transform.worldPresent.getPos();
    std::tie(xPosPast, yPosPast) = transform.worldPast.getPos();
    /*xPosPres -= bounds.minX; yPosPres -= bounds.minY;
    xPosPast -= bounds.minX; yPosPast -= bounds.minY;*/
    float xDiffPres = xPres - xPosPres, yDiffPres = yPres - yPosPres;
    float xDiffPast = xPres - xPosPast, yDiffPast = yPres - yPosPast;

    transform.worldPresent.translate(xDiffPres, yDiffPres);
    transform.local.translate(xDiffPres, yDiffPres);

    transform.worldPast.translate(xDiffPast, yDiffPast);

    //tfGraph.setDirty(id);

    auto& parent = tfGraph.transforms[id];
    std::vector<uint32_t> children{parent.children};

    for(auto child : children) {
        auto& tfNode = tfGraph.transforms[child];
        children.insert(children.end(), tfNode.children.begin(), tfNode.children.end());
        if (hasEntity[child]) {
            auto height = nodeHeights[child];
            Transform& tf = tfPool.lock()->operator[](entities[height][entityIDXs[child]].first->index).data;
            tf.worldPresent.translate(xDiffPres, yDiffPres);
            tf.worldPast.translate(xDiffPast, yDiffPast);

        } else {
            auto childEntity = manager->getEntity(child);
            auto tfComp = childEntity->components.find("transform");
            if (tfComp != childEntity->components.end()) {
                Transform& tf = tfPool.lock()->operator[](tfComp->second.index).data;
                float xPos = 0, yPos = 0;
                tf.worldPresent.translate(xDiffPres, yDiffPres);
                tf.worldPast.translate(xDiffPast, yDiffPast);
            }
        }
    }


}
