#include "EntityManager.h"
#include <iostream>
#include <algorithm>

EntityManager::EntityManager() : tagManager{this}, groupManager{this} {

    for ( auto& factoryPair : componentUtils::factoryMap ) {
        factoryPair.second->registerManager(this);
    }

}

EntityManager::~EntityManager() {
    std::vector<uint32_t> entityIDs;
    for (auto& entity : entities ) {
        entityIDs.push_back(entity.first);
    }
    for (auto entity: entityIDs) {
        destroyEntity(entity);
    }
    for ( auto& factoryPair : componentUtils::factoryMap ) {
        factoryPair.second->deregisterManager(this);
    }
    for (auto& system : systems) {
        system = nullptr;
    }
}

void EntityManager::update(float dt) {
    for (auto system : systems) {
        system->process(dt);
    }
    for (uint32_t entity : entitiesToRefresh) {
        refreshEntity(entity);
    }
    entitiesToRefresh.clear();
    for (uint32_t entity : entitiesToDestroy) {
        destroyEntity(entity);
    }
    entitiesToDestroy.clear();
}

void EntityManager::addSystem(EntitySystem* system) {
    if (system) {
        systems.push_back(system);
    }
    std::sort(systems.begin(), systems.end(), [](const EntitySystem* a, const EntitySystem* b){return a->priority < b->priority;});;
}

uint32_t EntityManager::createEntity() {

    uint32_t id;
    if (!freeIDs.empty()) {
        id = freeIDs.back();
        freeIDs.pop_back();
    } else
        id = createEntity(0);
    entities[id];
    return id;
}

uint32_t EntityManager::createEntity(uint32_t id) {
    if (!freeIDs.empty() && entities.find(id) != entities.end()) {
        id = freeIDs.back();
        freeIDs.pop_back();
    } else
        while (entities.find(id) != entities.end()) ++id;
    entities[id];
    return id;
}

void EntityManager::destroyEntity(uint32_t id) {
    std::unordered_map<std::string, component_pair>& components = entities[id];
    for(auto compPair : components) {
        freeComponents[compPair.first].push_back(compPair.second.second);
    }
    for (auto system : systems) {
        system->removeEntity(id);
    }
    entities.erase(id);
    freeIDs.push_back(id);
    tagManager.untagEntity(id);
    groupManager.ungroupEntity(id);
    removeParent(id);
    clearChildren(id);
}

void EntityManager::addComponent(std::string& instructions, uint32_t id) {

    std::string::size_type colon = instructions.find_first_of(':');
    std::string::size_type endName = instructions.find_first_of(" \n\r\f\t\v", colon);
    std::string compName = instructions.substr(colon + 1, endName - colon - 1);

    if (entities.find(id) == entities.end()) id = createEntity(id);
    auto& entity = entities[id];
    auto componentID = entity.find(compName);
    auto& factory = componentUtils::factoryMap.at(compName);
    if (componentID == entity.end() ) {
        if (freeComponents[compName].empty()) {
            entities[id][compName] = component_pair{true, factory->build(this, instructions)};
        }
        else {
            component_pair compPair{true, freeComponents[compName].front()};
            entity[compName] = compPair;
            freeComponents[compName].pop_front();
            factory->build(this, compPair.second, instructions);
        }
    }
    else {
        componentID->second.first = true;
        factory->build(this, componentID->second.second, instructions);
    }

    entitiesToRefresh.insert(id);
}

void EntityManager::addComponent(std::string&& instructions, uint32_t id) {

    std::string::size_type colon = instructions.find_first_of(':');
    std::string::size_type endName = instructions.find_first_of(" \n\r\f\t\v", colon);
    std::string compName = instructions.substr(colon + 1, endName - colon - 1);

    if (entities.find(id) == entities.end()) id = createEntity(id);
    auto& entity = entities[id];

    auto componentID = entity.find(compName);
    auto& factory = componentUtils::factoryMap.at(compName);
    if (componentID == entity.end() ) {
        if (freeComponents[compName].empty()) {
            entities[id][compName] = component_pair{true, factory->build(this, instructions)};
        }
        else {
            entity[compName] = component_pair{true, freeComponents[compName].front()};
            freeComponents[compName].pop_front();
            entities[id][compName] = component_pair{true, factory->build(this, componentID->second.second, instructions)};
        }
    }
    else {
        componentID->second.first = true;
        factory->build(this, componentID->second.second, instructions);
    }

    entitiesToRefresh.insert(id);
}

void EntityManager::removeComponent(std::string& compName, uint32_t id) {
    auto entity = entities.find(id);
    if (entity != entities.end()) {
        auto componentID = entity->second.find(compName);
        if (componentID != entity->second.end())
            componentID->second.first = false;

        bool alive = false;
        for (auto it = entity->second.begin(); !alive && it != entity->second.end(); ++it) {
            alive = it->second.first;
        }
        if (!alive) entitiesToDestroy.insert(id);
        else entitiesToRefresh.insert(id);
    }
}

void EntityManager::removeComponent(std::string&& compName, uint32_t id) {

    auto entity = entities.find(id);
    if (entity != entities.end()) {
        auto componentID = entity->second.find(compName);
        if (componentID != entity->second.end())
            componentID->second.first = false;

        bool alive = false;
        for (auto it = entity->second.begin(); !alive && it != entity->second.end(); ++it) {
            alive = it->second.first;
        }
        if (!alive) entitiesToDestroy.insert(id);
        else entitiesToRefresh.insert(id);
    }
}

EntityManager::entity_map const * EntityManager::getEntity(uint32_t id) {
    EntityManager::entity_map const * entitySet = nullptr;
    auto entity = entities.find(id);
    if (entity != entities.end())
        entitySet = &entity->second;
    return entitySet;
}

void EntityManager::refreshEntity(uint32_t id) {
    for (auto& system : systems) {
        system->refreshEntity(id);
    }
}

void EntityManager::setParent(uint32_t child, uint32_t parent) {
    auto getParent = childToParent.find(child);
    if (getParent != childToParent.end()) {
        if (getParent->second != parent)
            removeParent(child);
    }
    childToParent[child] = parent;
    auto& children = parentToChildren[parent];
    auto it = std::find(children.begin(), children.end(), child);
    if (it == children.end()) {
        children.push_back(child);
    }
}

void EntityManager::addChild(uint32_t parent, uint32_t child) {
    auto getParent = childToParent.find(child);
    if (getParent != childToParent.end()) {
        if (getParent->second != parent)
            removeParent(child);
    }
    childToParent[child] = parent;
    auto& children = parentToChildren[parent];
    auto it = std::find(children.begin(), children.end(), child);
    if (it == children.end()) {
        children.push_back(child);
    }
}

void EntityManager::removeParent(uint32_t child) {
    uint32_t parent = childToParent[child];
    auto& children = parentToChildren[parent];
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        *it = children.back();
        children.pop_back();
    }
    childToParent.erase(child);

}

void EntityManager::removeChild(uint32_t parent, uint32_t child) {
    auto& children = parentToChildren[parent];
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        *it = children.back();
        children.pop_back();
    }
    childToParent.erase(child);

}

void EntityManager::clearChildren(uint32_t parent) {
    auto& children = parentToChildren[parent];
    for (uint32_t child : children) {
        childToParent.erase(child);
    }
    parentToChildren.erase(parent);
}

uint32_t EntityManager::getParent(uint32_t child) {
    return childToParent[child];
}

std::vector<uint32_t>& EntityManager::getChildren(uint32_t parent) {
    return parentToChildren[parent];
}

