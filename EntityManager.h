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

    friend TagManager;
    friend GroupManager;

public:

    struct ComponentHandle {

        bool active;
        bool dirty;

        std::size_t index;

        ComponentHandle();

        void setHandle(bool act, bool dir, std::size_t idx);
    };

    typedef std::map<std::string, ComponentHandle> entity_map;

    EntityManager();
    ~EntityManager();

    void update(float dt);

    void addSystem(EntitySystem* system);

    uint32_t createEntity();

    void destroyEntity(uint32_t id);

    std::vector<std::string> tokenize(std::string& instructions);

    std::vector<std::string> tokenize(std::string&& instructions);

    void addComponent(std::string& instructions, uint32_t id);

    void addComponent(std::string&& instructions, uint32_t id);

    void addComponent(std::vector<std::string>& instructions, uint32_t id);

    void addComponent(std::vector<std::string>&& instructions, uint32_t id);

    template<typename CMPType> void addComponent(CMPType& comp, uint32_t id);

    template<typename CMPType> void addComponent(CMPType&& comp, uint32_t id);

    template<typename CMPType> void removeComponent(uint32_t id);

    void removeComponent(std::string& cmpName, uint32_t id);

    void removeComponent(std::string&& cmpName, uint32_t id);

    entity_map const * getEntity(uint32_t id);

    template<typename CMPType> std::deque<CMPType>* getComponentPool();

    void setParent(uint32_t child, uint32_t parent);

    void addChild(uint32_t parent, uint32_t child);

    void removeParent(uint32_t child);

    void removeChild(uint32_t parent, uint32_t child);

    void clearChildren(uint32_t parent);

    uint32_t getParent(uint32_t child);

    std::vector<uint32_t>& getChildren(uint32_t parent);

    TagManager tagManager;
    GroupManager groupManager;

    //  deferred system adding/removing of entities for post-update
    std::vector<uint32_t> entitiesToDestroy;
    std::vector<uint32_t> entitiesToRefresh;

    void excludeFromRefresh(uint32_t id);

    void forceRefresh(uint32_t id);

protected:

private:

    void eraseEntity(uint32_t id);

    void refreshEntity(uint32_t id);
    //  mapping from entity id to an entity, basically a map of components
    std::vector<entity_map> entities;

    std::vector<uint8_t> isAlive;

    std::size_t aliveCount;

    std::vector<uint8_t> toRefresh;

    std::vector<uint8_t> toDestroy;

    //  recycles entity ids from destroyed entities
    std::vector<uint32_t> freeIDs;
    //  recycles component indexes from destroyed components
    std::map<std::string, std::deque<std::size_t>> freeComponents;
    std::vector<EntitySystem*> systems;
    //  maps for two-way entity relationships
    std::unordered_map<uint32_t, uint32_t> childToParent;
    std::unordered_map<uint32_t, std::vector<uint32_t>> parentToChildren;


};

template<typename CMPType> void EntityManager::addComponent(CMPType& comp, uint32_t id) {

    const std::string& cmpName = CMPType::getName();

    auto& entity = entities[id];
    auto componentID = entity.find(cmpName);
    auto& factory = componentUtils::factoryMap.at(cmpName);
    if (componentID == entity.end() ) {
        if (freeComponents[cmpName].empty()) {
            entities[id][cmpName].setHandle( true, true, factory->build(this, &comp));
        }
        else {
            auto front = freeComponents[cmpName].front();
            freeComponents[cmpName].pop_front();
            factory->build(this, front, &comp);
            entities[id][cmpName].setHandle( true, true, front);
        }
    }
    else {
        componentID->second.active = true;
        componentID->second.dirty = true;
        factory->build(this, componentID->second.index, &comp);
    }

    if (!toRefresh[id]) {
        entitiesToRefresh.emplace_back(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

template<typename CMPType> void EntityManager::addComponent(CMPType&& comp, uint32_t id) {

    const std::string& cmpName = CMPType::getName();

    auto& entity = entities[id];
    auto componentID = entity.find(cmpName);
    auto& factory = componentUtils::factoryMap.at(cmpName);
    if (componentID == entity.end() ) {
        if (freeComponents[cmpName].empty()) {
            entities[id][cmpName].setHandle( true, true, factory->build(this, &comp));
        }
        else {
            auto front = freeComponents[cmpName].front();
            freeComponents[cmpName].pop_front();
            factory->build(this, front, &comp);
            entities[id][cmpName].setHandle( true, true, front);
        }
    }
    else {
        componentID->second.active = true;
        componentID->second.dirty = true;
        factory->build(this, componentID->second.index, &comp);
    }

    if (!toRefresh[id]) {
        entitiesToRefresh.emplace_back(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

template<typename CMPType> void EntityManager::removeComponent(uint32_t id) {
    const std::string& cmpName = CMPType::getName();
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
        toRefresh[id] = false;
    }
    else if (!toRefresh[id]) {
        entitiesToRefresh.emplace_back(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }

}

template<typename CMPType> std::deque<CMPType>* EntityManager::getComponentPool() {
    return &CMPType::componentPools[this];

}

#endif // ENTITYMANAGER_H_INCLUDED
