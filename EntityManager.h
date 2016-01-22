#ifndef ENTITYMANAGER_H_INCLUDED
#define ENTITYMANAGER_H_INCLUDED

#include <cstdint>
#include <unordered_map>
#include <deque>
#include "component.h"
#include "TagManager.h"
#include "GroupManager.h"
#include "EntitySystem.h"

class EntitySystem;

class EntityManager {

public:

    typedef std::pair<bool, std::size_t> component_pair;
    typedef std::unordered_map<std::string, component_pair> entity_map;

    EntityManager();
    ~EntityManager();

    void update(float dt);

    void addSystem(EntitySystem* system);

    uint32_t createEntity();

    uint32_t createEntity(uint32_t id);

    void destroyEntity(uint32_t id);

    void addComponent(std::string& instructions, uint32_t id);

    void addComponent(std::string&& instructions, uint32_t id);

    template<typename CMPType> void addComponent(CMPType& comp, uint32_t id);

    template<typename CMPType> void addComponent(CMPType&& comp, uint32_t id);

    template<typename CMPType> void removeComponent(uint32_t id);

    void removeComponent(std::string& compName, uint32_t id);

    void removeComponent(std::string&& compName, uint32_t id);

    entity_map const * getEntity(uint32_t id);

    template<typename CMPType> std::vector<CMPType>* const getComponentPool();

    TagManager tagManager;
    GroupManager groupManager;

protected:

private:
    const uint32_t compTypeCount;
    std::unordered_map<uint32_t, entity_map> entities;
    std::vector<uint32_t> freeIDs;
    std::unordered_map<std::string, std::deque<std::size_t>> freeComponents;
    std::vector<EntitySystem*> systems;

};

template<typename CMPType> void EntityManager::addComponent(CMPType& comp, uint32_t id) {

    std::string compName = CMPType::getName();
    if (entities.find(id) == entities.end()) id = createEntity(id);
    auto& entity = entities[id];
    auto componentID = entity.find(compName);
    auto factory = componentUtils::factoryMap.at(compName);
    if (componentID == entity.end() ) {
        if (freeComponents[compName].empty()) {
            entity[compName] = component_pair{true, CMPType::componentPools[this].size()};
            factory->build(this, comp);
        } else {
            entity[compName] = component_pair{true, freeComponents[compName].front()};
            freeComponents[compName].pop_front();
            factory->build(this, componentID->second.second, comp);
        }
    } else {
        componentID->first = true;
        factory->build(this, componentID->second.second, comp);
    }

}

template<typename CMPType> void EntityManager::addComponent(CMPType&& comp, uint32_t id) {

    std::string compName = CMPType::getName();
    if (entities.find(id) == entities.end()) id = createEntity(id);
    auto& entity = entities[id];
    auto componentID = entity.find(compName);
    auto factory = componentUtils::factoryMap.at(compName);
    if (componentID == entity.end() ) {
        if (freeComponents[compName].empty()) {
            entity[compName] = component_pair{true, CMPType::componentPools[this].size()};
            factory->build(this, comp);
        } else {
            entity[compName] = component_pair{true, freeComponents[compName].front()};
            freeComponents[compName].pop_front();
            factory->build(this, componentID->second.second, comp);
        }
    } else {
        componentID->first = true;
        factory->build(this, componentID->second.second, comp);
    }

}

template<typename CMPType> void EntityManager::removeComponent(uint32_t id) {
    std::string compName = CMPType::getName();
    auto entity = entities.find(id);
    if (entity != entities.end()) {
        auto componentID = entity->second.find(compName);
        if (componentID != entity->second.end())
            componentID->second.first = false;
    }

}

template<typename CMPType> std::vector<CMPType>* const EntityManager::getComponentPool() {
    return &CMPType::componentPools.at(this);

}

#endif // ENTITYMANAGER_H_INCLUDED
