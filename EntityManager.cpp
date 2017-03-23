#include "EntityManager.h"
#include "SDL.h"
#include <iostream>
#include <algorithm>
#include <sstream>

EntityManager::EntityManager() : tagManager{this}, groupManager{this} {

    for ( auto& factoryPair : componentUtils::factoryMap ) {
        factoryPair.second->registerManager(this);
    }
    using componentUtils::factoryMap;

    entities.reserve(1 << 16);
    isAlive.reserve(1 << 16);
    toRefresh.reserve(1 << 16);
    toDestroy.reserve(1 << 16);
    aliveCount = 0;

}

EntityManager::~EntityManager() {
    using componentUtils::factoryMap;
    for ( auto& factoryPair : factoryMap ) {
        factoryPair.second->deregisterManager(this);
    }
}

void EntityManager::update(float dt) {
    auto beg = SDL_GetPerformanceCounter();
    for (uint32_t entity : entitiesToRefresh) {
        if (toRefresh[entity] == true) {
            refreshEntity(entity);
            toRefresh[entity] = false;
        }
        for (auto& component: entities[entity].components) {
            component.second.dirty = false;
        }
    }
    auto ed = SDL_GetPerformanceCounter();

    //std::cout << "REFRESH - " << ((ed - beg) * 1000.f / SDL_GetPerformanceFrequency()) << '\t' << entitiesToRefresh.size() << " entities" << std::endl;
    entitiesToRefresh.clear();
    beg = SDL_GetPerformanceCounter();
    for (uint32_t entity : entitiesToDestroy) {
        if (entity >= toDestroy.size()) {
            std::cout << "Inappropriate Deletion: " << entity << '\n';
        }
        if (toDestroy[entity])
            eraseEntity(entity); {
            toDestroy[entity] = false;
        }
    }
    ed = SDL_GetPerformanceCounter();

    //std::cout << "DESTROY - " << ((ed - beg) * 1000.f / SDL_GetPerformanceFrequency()) << '\t' << entitiesToDestroy.size() << " entities" << std::endl;
    entitiesToDestroy.clear();
    beg = SDL_GetPerformanceCounter();
    for (auto system : systems) {
        system->process(dt);
    }
    ed = SDL_GetPerformanceCounter();
    //std::cout << "PROCESSING - " << ((ed - beg) * 1000.f / SDL_GetPerformanceFrequency()) << '\t' << entities.size() << ',' << aliveCount << " entities" << std::endl;
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
    entity_map& components = entities[id].components;
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
}

void EntityManager::destroyEntity(uint32_t id) {
    if (id >= entities.size()) {
        std::cout << "Inappropriate Deletion: " << id;
        return;
    }
    if (!isAlive[id] || toDestroy[id]){
        return;
    }
    entitiesToDestroy.insert(id);
    toDestroy[id] = true;
    toRefresh[id] = false;
}

void EntityManager::addComponent(std::string cmpName, sol::object comp, uint32_t id) {

    auto& entity = entities.at(id);
    auto componentID = entity.components.find(cmpName);
    auto& factory = componentUtils::factoryMap.at(cmpName);
    if (componentID == entity.components.end() ) {
        if (freeComponents[cmpName].empty()) {
            entities[id].components[cmpName].setHandle( true, true, factory->build(this, comp));
        }
        else {
            auto front = freeComponents[cmpName].front();
            freeComponents[cmpName].pop_front();
            factory->build(this, front, comp);
            entities[id].components[cmpName].setHandle( true, true, front);
        }
    }
    else {
        componentID->second.active = true;
        componentID->second.dirty = true;
        factory->build(this, componentID->second.index, comp);
    }
    if (!toRefresh[id]) {
        entitiesToRefresh.emplace(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

void EntityManager::removeComponent(std::string& cmpName, uint32_t id) {
    auto& entity = entities[id];
    auto componentID = entity.components.find(cmpName);
    if (componentID != entity.components.end()) {
        componentID->second.active = false;
        componentID->second.dirty = true;
    }

    bool alive = false;
    for (auto it = entity.components.begin(); !alive && it != entity.components.end(); ++it) {
        alive = it->second.active;
    }
    if (!alive) {
        destroyEntity(id);
    }
    else if (!toRefresh[id]) {
        entitiesToRefresh.insert(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

void EntityManager::removeComponent(std::string&& cmpName, uint32_t id) {
    auto& entity = entities[id];
    auto componentID = entity.components.find(cmpName);
    if (componentID != entity.components.end()) {
        componentID->second.active = false;
        componentID->second.dirty = true;
    }

    bool alive = false;
    for (auto it = entity.components.begin(); !alive && it != entity.components.end(); ++it) {
        alive = it->second.active;
    }
    if (!alive) {
        destroyEntity(id);
    }
    else if (!toRefresh[id]) {
        entitiesToRefresh.insert(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

EntityManager::EntityInfo const * EntityManager::getEntity(uint32_t id) {
    return (id < entities.size() && isAlive[id]) ? &entities[id] : nullptr;
}

void EntityManager::refreshEntity(uint32_t id) {
    for (auto& system : systems) {
        system->refreshEntity(id);
    }
}

void EntityManager::excludeFromRefresh(uint32_t id) {
    toRefresh[id] = 2;
}

void EntityManager::forceRefresh(uint32_t id) {
    toRefresh[id] = true;
    entitiesToRefresh.insert(id);
}

void EntityManager::registerWithMessage(Message::Type type, std::function<bool(Message&)>& listener, uint16_t priority) {
    messageMap[type][priority].emplace_back(&listener);
}

void EntityManager::deregisterFromMessage(Message::Type type, std::function<bool(Message&)>& listener, uint16_t priority) {
    auto& mVec = messageMap[type][priority];
    mVec.erase(std::remove(mVec.begin(), mVec.end(), &listener), mVec.end());
}

void EntityManager::sendMessage(Message& message) {
    auto& listenerPriorities = messageMap[message.type];
    for (auto& listeners : listenerPriorities) {
        for (auto& listener : listeners.second) {
            if (!listener->operator()(message)) return;
        }
    }

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

void EntityManager::EntityInfo::getComponent(std::string cmpName, sol::table cmp, EntityManager &manager) {
    auto it = components.find(cmpName);
    if (it == components.end() || !it->second.active)
        return;
    auto &factory = componentUtils::factoryMap.at(cmpName);
    factory->getComponent(manager, cmp, it->second.index);

}
