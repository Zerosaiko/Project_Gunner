#include "EntityManager.h"
#include <iostream>
#include <algorithm>

EntityManager::EntityManager() : compTypeCount{componentUtils::factoryMap.size()} {



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
    return id;
}

uint32_t EntityManager::createEntity(uint32_t id) {
    if (!freeIDs.empty() && entities.find(id) != entities.end()) {
        id = freeIDs.back();
        freeIDs.pop_back();
    }
    while (entities.find(id) != entities.end()) id = (id+1) << 1;
    return id;
}

void EntityManager::destroyEntity(uint32_t id) {
    std::unordered_map<std::string, component_pair>& components = entities[id];
    for(auto compPair : components) {
        freeComponents[compPair.first].push_back(compPair.second.second);
    }
    entities.erase(id);
    freeIDs.push_back(id);
    tagManager.untagEntity(id);
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
            entity[compName] = component_pair{true, freeComponents[compName].front()};
            freeComponents[compName].pop_front();
            entities[id][compName] = component_pair{true, factory->build(this, componentID->second.second, instructions)};
        }
    }
    else {
        componentID->second.first = true;
        factory->build(this, componentID->second.second, instructions);
    }
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
}

void EntityManager::removeComponent(std::string& compName, uint32_t id) {
    auto entity = entities.find(id);
    if (entity != entities.end()) {
        auto componentID = entity->second.find(compName);
        if (componentID != entity->second.end())
            componentID->second.first = false;
    }
}

void EntityManager::removeComponent(std::string&& compName, uint32_t id) {

    auto entity = entities.find(id);
    if (entity != entities.end()) {
        auto componentID = entity->second.find(compName);
        if (componentID != entity->second.end())
            componentID->second.first = false;
    }
}

EntityManager::entity_map const * EntityManager::getEntity(uint32_t id) {
    EntityManager::entity_map const * entitySet = nullptr;
    auto entity = entities.find(id);
    if (entity != entities.end())
        entitySet = &entity->second;
    return entitySet;
}
