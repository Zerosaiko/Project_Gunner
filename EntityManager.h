#ifndef ENTITYMANAGER_H_INCLUDED
#define ENTITYMANAGER_H_INCLUDED

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include "component.h"
#include "TagManager.h"
#include "GroupManager.h"
#include "EntitySystem.h"

class EntitySystem;

// intermediates between systems and components, making entities composed of a variable number of components
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

    template<typename CMPType> std::vector<CMPType>* getComponentPool();

    void refreshEntity(uint32_t id);

    void setParent(uint32_t child, uint32_t parent);

    void addChild(uint32_t parent, uint32_t child);

    void removeParent(uint32_t child);

    void removeChild(uint32_t parent, uint32_t child);

    void clearChildren(uint32_t parent);

    uint32_t getParent(uint32_t child);

    std::vector<uint32_t>& getChildren(uint32_t parent);

    TagManager tagManager;
    GroupManager groupManager;

    //  deferred system adding of entities for post-update
    std::unordered_set<uint32_t> entitiesToDestroy;
    std::unordered_set<uint32_t> entitiesToRefresh;

protected:

private:
    //  mapping from entity id to an entity, basically a map of components
    std::unordered_map<uint32_t, entity_map> entities;
    //  recycles entity ids from destroyed entities
    std::vector<uint32_t> freeIDs;
    //  recycles component indexes from destroyed components
    std::unordered_map<std::string, std::deque<std::size_t>> freeComponents;
    std::vector<EntitySystem*> systems;
    //  maps for two-way entity relationships
    std::unordered_map<uint32_t, uint32_t> childToParent;
    std::unordered_map<uint32_t, std::vector<uint32_t>> parentToChildren;


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
            factory->build(this, &comp);
        } else {
            entity[compName] = component_pair{true, freeComponents[compName].front()};
            freeComponents[compName].pop_front();
            factory->build(this, componentID->second.second, &comp);
        }
    } else {
        componentID->second.first = true;
        factory->build(this, componentID->second.second, &comp);
    }
    entitiesToRefresh.insert(id);
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
            factory->build(this, &comp);
        } else {
            entity[compName] = component_pair{true, freeComponents[compName].front()};
            freeComponents[compName].pop_front();
            factory->build(this, componentID->second.second, &comp);
        }
    } else {
        componentID->first = true;
        factory->build(this, componentID->second.second, &comp);
    }
    entitiesToRefresh.insert(id);

}

template<typename CMPType> void EntityManager::removeComponent(uint32_t id) {
    std::string compName = CMPType::getName();
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

template<typename CMPType> std::vector<CMPType>* EntityManager::getComponentPool() {
    return &CMPType::componentPools[this];

}

#endif // ENTITYMANAGER_H_INCLUDED
