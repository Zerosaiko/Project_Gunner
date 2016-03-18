#include "EntityManager.h"
#include "SDL.h"
#include <iostream>
#include <algorithm>

EntityManager::EntityManager() : tagManager{this}, groupManager{this} {

    for ( auto& factoryPair : componentUtils::factoryMap ) {
        factoryPair.second->registerManager(this);
    }

    entities.reserve(1 << 16);
    isAlive.reserve(1 << 16);
    toRefresh.reserve(1 << 16);
    toDestroy.reserve(1 << 16);
    aliveCount = 0;

}

EntityManager::~EntityManager() {
    for ( auto& factoryPair : componentUtils::factoryMap ) {
        factoryPair.second->deregisterManager(this);
    }
}

void EntityManager::update(float dt) {
    auto beg = SDL_GetPerformanceCounter();
    for (auto system : systems) {
        system->process(dt);
    }
    auto ed = SDL_GetPerformanceCounter();
    std::cout << "PROCESSING - " << ((ed - beg) * 1000.f / SDL_GetPerformanceFrequency()) << '\t' << entities.size() << ',' << aliveCount << " entities" << std::endl;
    beg = SDL_GetPerformanceCounter();
    for (uint32_t entity : entitiesToRefresh) {
        if (toRefresh[entity] == true) {
            refreshEntity(entity);
            toRefresh[entity] = false;
        }
        for (auto& component: entities[entity]) {
            component.second.dirty = false;
        }
    }
    ed = SDL_GetPerformanceCounter();
    std::cout << "REFRESH - " << ((ed - beg) * 1000.f / SDL_GetPerformanceFrequency()) << '\t' << entitiesToRefresh.size() << " entities" << std::endl;
    entitiesToRefresh.clear();
    beg = SDL_GetPerformanceCounter();
    for (uint32_t entity : entitiesToDestroy) {
        if (toDestroy[entity])
            eraseEntity(entity);
        toDestroy[entity] = false;
    }
    ed = SDL_GetPerformanceCounter();
    std::cout << "DESTROY - " << ((ed - beg) * 1000.f / SDL_GetPerformanceFrequency()) << '\t' << entitiesToDestroy.size() << " entities" << std::endl;
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
        isAlive[id] = true;
    } else {
        id = entities.size();
        entities.emplace_back();
        isAlive.emplace_back(true);
    }
    ++aliveCount;
    toRefresh.emplace_back(false);
    toDestroy.emplace_back(false);
    return id;
}

void EntityManager::eraseEntity(uint32_t id) {
    if (!isAlive[id]) return;
    for (auto& system : systems) {
        system->removeEntity(id);
    }
    entity_map& components = entities[id];
    for(auto& compPair : components) {
        freeComponents[compPair.first].push_back(compPair.second.index);
    }
    components.clear();
    isAlive[id] = false;
    --aliveCount;
    toRefresh[id] = false;
    toDestroy[id] = false;
    freeIDs.push_back(id);
    tagManager.untagEntity(id);
    groupManager.ungroupEntity(id);
    removeParent(id);
    clearChildren(id);
}

void EntityManager::destroyEntity(uint32_t id) {
    if (!isAlive[id] || toDestroy[id]) return;
    entitiesToDestroy.emplace_back(id);
    toDestroy[id] = true;
    toRefresh[id] = false;
}

std::vector<std::string> EntityManager::tokenize(std::string& instructions) {

    std::string::size_type colon = instructions.find_first_of(':');
    std::string::size_type endName = instructions.find_first_of(" \n\r\f\t\v", colon);
    std::string cmpName = instructions.substr(colon + 1, endName - colon - 1);

    auto& factory = componentUtils::factoryMap.at(cmpName);

    return factory->tokenize(instructions);

}

std::vector<std::string> EntityManager::tokenize(std::string&& instructions) {

    std::string::size_type colon = instructions.find_first_of(':');
    std::string::size_type endName = instructions.find_first_of(" \n\r\f\t\v", colon);
    std::string cmpName = instructions.substr(colon + 1, endName - colon - 1);

    auto& factory = componentUtils::factoryMap.at(cmpName);

    return factory->tokenize(instructions);

}

void EntityManager::addComponent(std::string& instructions, uint32_t id) {
    addComponent(tokenize(instructions), id);
}

void EntityManager::addComponent(std::string&& instructions, uint32_t id) {

    addComponent(tokenize(instructions), id);
}

void EntityManager::addComponent(std::vector<std::string>& instructions, uint32_t id) {

    auto& cmpName = instructions.at(1);

    auto& entity = entities[id];
    auto componentID = entity.find(cmpName);
    auto& factory = componentUtils::factoryMap.at(cmpName);
    if (componentID == entity.end() ) {
        if (freeComponents[cmpName].empty()) {
            entities[id][cmpName].setHandle( true, true, factory->build(this, instructions));
        }
        else {
            auto front = freeComponents[cmpName].front();
            freeComponents[cmpName].pop_front();
            factory->build(this, front, instructions);
            entities[id][cmpName].setHandle( true, true, front);
        }
    }
    else {
        componentID->second.active = true;
        componentID->second.dirty = true;
        factory->build(this, componentID->second.index, instructions);
    }
    if (!toRefresh[id]) {
        entitiesToRefresh.emplace_back(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}
void EntityManager::addComponent(std::vector<std::string>&& instructions, uint32_t id) {

    auto& cmpName = instructions.at(1);

    auto& entity = entities[id];
    auto componentID = entity.find(cmpName);
    auto& factory = componentUtils::factoryMap.at(cmpName);
    if (componentID == entity.end() ) {
        if (freeComponents[cmpName].empty()) {
            entities[id][cmpName].setHandle( true, true, factory->build(this, instructions));
        }
        else {
            auto front = freeComponents[cmpName].front();
            freeComponents[cmpName].pop_front();
            factory->build(this, front, instructions);
            entities[id][cmpName].setHandle( true, true, front);
        }
    }
    else {
        componentID->second.active = true;
        componentID->second.dirty = true;
        factory->build(this, componentID->second.index, instructions);
    }
    if (!toRefresh[id]) {
        entitiesToRefresh.emplace_back(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

void EntityManager::removeComponent(std::string& cmpName, uint32_t id) {
    auto& entity = entities[id];
    auto componentID = entity.find(cmpName);
    if (componentID != entity.end()) {
        componentID->second.active = false;
        componentID->second.dirty = true;
    }

    bool alive = false;
    for (auto it = entity.begin(); !alive && it != entity.end(); ++it) {
        alive = it->second.active;
    }
    if (!alive) {
        destroyEntity(id);
    }
    else if (!toRefresh[id]) {
        entitiesToRefresh.emplace_back(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

void EntityManager::removeComponent(std::string&& cmpName, uint32_t id) {
    auto& entity = entities[id];
    auto componentID = entity.find(cmpName);
    if (componentID != entity.end()) {
        componentID->second.active = false;
        componentID->second.dirty = true;
    }

    bool alive = false;
    for (auto it = entity.begin(); !alive && it != entity.end(); ++it) {
        alive = it->second.active;
    }
    if (!alive) {
        destroyEntity(id);
    }
    else if (!toRefresh[id]) {
        entitiesToRefresh.emplace_back(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

EntityManager::entity_map const * EntityManager::getEntity(uint32_t id) {
    EntityManager::entity_map const * entitySet = nullptr;
    if (isAlive[id])
        entitySet = &entities[id];
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

void EntityManager::excludeFromRefresh(uint32_t id) {
    toRefresh[id] = 2;
}

void EntityManager::allowRefresh(uint32_t id) {
    toRefresh[id] = true;
    entitiesToRefresh.emplace_back(id);
}

EntityManager::ComponentHandle::ComponentHandle() {
    active = false;
    dirty = false;

    index = 0;

}

void EntityManager::ComponentHandle::setHandle(bool act, bool dir, std::size_t idx) {
    active = act;
    dirty = dir;
    index = idx;

}
