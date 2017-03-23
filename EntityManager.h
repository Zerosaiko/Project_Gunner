#ifndef ENTITYMANAGER_H_INCLUDED
#define ENTITYMANAGER_H_INCLUDED

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <functional>
#include "component.h"
#include "TagManager.h"
#include "GroupManager.h"
#include "EntitySystem.h"
#include "Message.h"
#include "sol.hpp"

class EntitySystem;

// intermediates between systems and components, making entities composed of a variable number of components
class EntityManager {

    friend TagManager;
    friend GroupManager;

    template<typename CMPType>
    friend void addComponent(EntityManager& manager, CMPType& comp, uint32_t id);

    template<typename CMPType>
    friend void addComponent(EntityManager& manager, CMPType&& comp, uint32_t id);

    template<typename CMPType>
    friend void removeComponent(EntityManager& manager, uint32_t id);
public:

    struct ComponentHandle {

        bool active;
        bool dirty;

        std::size_t index;

        ComponentHandle();

        void setHandle(bool act, bool dir, std::size_t idx);
    };


    typedef std::map<std::string, ComponentHandle> entity_map;

    struct EntityInfo {

        entity_map components;

        void getComponent(std::string cmpName, sol::table cmp, EntityManager &manager);

    };

    EntityManager();
    ~EntityManager();

    void update(float dt);

    void addSystem(EntitySystem* system);

    uint32_t createEntity();

    void destroyEntity(uint32_t id);

    template<typename CMPType> void addComponent(CMPType& comp, uint32_t id);

    template<typename CMPType> void addComponent(CMPType&& comp, uint32_t id);

    void addComponent(std::string cmpName, sol::object comp, uint32_t id);

    void removeComponent(std::string& cmpName, uint32_t id);

    void removeComponent(std::string&& cmpName, uint32_t id);

    template<typename CMPType> void removeComponent(uint32_t id);

    EntityInfo const * getEntity(uint32_t id);

    template<typename CMPType> std::weak_ptr<std::deque<CMPType>> getComponentPool();

    TagManager tagManager;
    GroupManager groupManager;

    //  deferred system adding/removing of entities for post-update
    std::unordered_set<uint32_t> entitiesToDestroy;
    std::unordered_set<uint32_t> entitiesToRefresh;

    void excludeFromRefresh(uint32_t id);

    void forceRefresh(uint32_t id);

    void registerWithMessage(Message::Type, std::function<bool(Message&)>&, uint16_t priority);

    void deregisterFromMessage(Message::Type, std::function<bool(Message&)>&, uint16_t priority);

    void sendMessage(Message& message);

protected:

private:

    void eraseEntity(uint32_t id);

    void refreshEntity(uint32_t id);
    //  mapping from entity id to an entity, basically a map of components
    std::vector<EntityInfo> entities;

    std::vector<uint8_t> isAlive;

    std::size_t aliveCount;

    std::vector<uint8_t> toRefresh;

    std::vector<uint8_t> toDestroy;

    //  recycles entity ids from destroyed entities
    std::vector<uint32_t> freeIDs;
    //  recycles component indexes from destroyed components
    std::map<std::string, std::deque<std::size_t>> freeComponents;

    std::deque<EntitySystem*> systems;

    std::map<Message::Type, std::map<uint16_t, std::vector<std::function<bool(Message&)>* > > > messageMap;


};

template<typename CMPType> void EntityManager::addComponent(CMPType& comp, uint32_t id) {

    const std::string& cmpName = CMPType::getName();

    auto& entity = entities[id];
    auto componentID = entity.components.find(cmpName);
    auto& factory = componentUtils::factoryMap.at(cmpName);
    if (componentID == entity.components.end() ) {
        if (freeComponents[cmpName].empty()) {
            entities[id].components[cmpName].setHandle( true, true, factory->build(this, &comp));
        }
        else {
            auto front = freeComponents[cmpName].front();
            freeComponents[cmpName].pop_front();
            factory->build(this, front, &comp);
            entities[id].components[cmpName].setHandle( true, true, front);
        }
    }
    else {
        componentID->second.active = true;
        componentID->second.dirty = true;
        factory->build(this, componentID->second.index, &comp);
    }

    if (!toRefresh[id]) {
        entitiesToRefresh.insert(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

template<typename CMPType> void EntityManager::addComponent(CMPType&& comp, uint32_t id) {

    const std::string& cmpName = CMPType::getName();

    auto& entity = entities[id];
    auto componentID = entity.components.find(cmpName);
    auto& factory = componentUtils::factoryMap.at(cmpName);
    if (componentID == entity.components.end() ) {
        if (freeComponents[cmpName].empty()) {
            entities[id].components[cmpName].setHandle( true, true, factory->build(this, &comp));
        }
        else {
            auto front = freeComponents[cmpName].front();
            freeComponents[cmpName].pop_front();
            factory->build(this, front, &comp);
            entities[id].components[cmpName].setHandle( true, true, front);
        }
    }
    else {
        componentID->second.active = true;
        componentID->second.dirty = true;
        factory->build(this, componentID->second.index, &comp);
    }

    if (!toRefresh[id]) {
        entitiesToRefresh.insert(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }
}

template<typename CMPType> void EntityManager::removeComponent(uint32_t id) {
    const std::string& cmpName = CMPType::getName();

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
        toRefresh[id] = false;
    }
    else if (!toRefresh[id]) {
        entitiesToRefresh.insert(id);
        toRefresh[id] = true;
        toDestroy[id] = false;
    }

}

template<typename CMPType> std::weak_ptr<std::deque<CMPType>> EntityManager::getComponentPool() {
    return CMPType::componentPools[this];

}

#endif // ENTITYMANAGER_H_INCLUDED
