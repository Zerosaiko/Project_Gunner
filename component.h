#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#include <cstdint>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <unordered_map>
#include <iostream>
#include <memory>
#include "componentName.h"
#include "sol.hpp"

class EntityManager;

class ComponentFactory;

namespace componentUtils {
    extern std::map<std::string, std::unique_ptr<ComponentFactory> > factoryMap;
}

// utility functions to turn parsed strings into objects
template <typename DataType>
DataType buildFromString(std::vector<std::string>& str, std::vector<std::string>::size_type& pos);

template <typename DataType>
DataType buildFromLua(sol::object& obj);

struct ComponentBase {
protected:
    ComponentBase();
    ~ComponentBase();

};

template <const std::string& cmpName, typename DataType>
struct Component;

//  doesn't seem to fit with the Factory pattern in retrospect, but can't think of any other names
//  basically container of functions to tokenize and create components
class ComponentFactory {
public:

    virtual std::size_t build(EntityManager* manager, size_t idx, ComponentBase* cmp) = 0;
    virtual std::size_t build(EntityManager* manager, size_t idx, sol::object& cmp) = 0;
    virtual std::size_t build(EntityManager* manager, ComponentBase* cmp) = 0;
    virtual std::size_t build(EntityManager* manager, sol::object& cmp) = 0;

    virtual void getComponent(EntityManager& manager, sol::table cmp, std::size_t index) = 0;

    virtual void registerManager(EntityManager*) = 0;

    virtual void deregisterManager(EntityManager*) = 0;

    virtual ~ComponentFactory() = 0;

protected:
    ComponentFactory();

};

//  genericish component class that can contain most datatypes for quick component making
//  the name must be a const string
//  because of the constructor that builds, DataType cannot be a vector of strings
//  DataType must have a default constructor and copy constructor
template <const std::string& cmpName, typename DataType>
struct Component : public ComponentBase {

private:

    class ComponentFactoryInternal : public ComponentFactory {

    public:

        std::size_t build(EntityManager* manager, size_t idx, ComponentBase* cmp);

        std::size_t build(EntityManager* manager, size_t idx, sol::object& cmp);

        std::size_t build(EntityManager* manager, ComponentBase* cmp);

        std::size_t build(EntityManager* manager, sol::object& cmp);

        virtual void getComponent(EntityManager& manager, sol::table cmp, std::size_t index);

        void registerManager(EntityManager*);

        void deregisterManager(EntityManager*);
    };

    typedef std::deque<Component<cmpName, DataType>> cmpPool;
public:
    static std::map<EntityManager*, std::shared_ptr<cmpPool>> componentPools;
    static const std::string getName();
    static std::unique_ptr<ComponentFactory> factory;

    //creates a ComponentFactory for the component type
    static void registerComponent();

    void build(std::vector<std::string> instructions);

    void build(sol::object& obj);

    Component() : data{} {}
    Component(const Component& other) : data(other.data) {}
    Component(const DataType& newData) : data(newData) {}
    Component(Component&& other) = default;
    Component(DataType&& newData) : data(newData) {}
    Component(sol::object& obj) : data{} {
        build(obj);
    }

    Component& operator=(const Component& other) { data = other.data; return *this; }

    DataType data;

};

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, size_t idx, ComponentBase* cmp) {
    auto& pool = *Component<cmpName, DataType>::componentPools[manager];
    pool[idx] = *static_cast<Component<cmpName, DataType>*>(cmp);
    return idx;
}

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, size_t idx, sol::object& cmp) {
    auto& pool = *Component<cmpName, DataType>::componentPools[manager];
    pool[idx].build(cmp);
    return idx;
}

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, ComponentBase* cmp) {
    auto& pool = *Component<cmpName, DataType>::componentPools[manager];
    pool.emplace_back(*static_cast<Component<cmpName, DataType>*>(cmp));
    return pool.size() - 1;
}

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, sol::object& cmp) {
    auto& pool = *Component<cmpName, DataType>::componentPools[manager];
    pool.emplace_back(cmp);
    return pool.size() - 1;
}


template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::ComponentFactoryInternal::getComponent(EntityManager& manager, sol::table cmp, std::size_t index) {
    auto& pool = *Component<cmpName, DataType>::componentPools[&manager];
    cmp["data"] = &pool[index].data;
}

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::ComponentFactoryInternal::registerManager(EntityManager* manager) {

    componentPools.emplace(manager, std::make_shared<cmpPool>(cmpPool()));

}

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::ComponentFactoryInternal::deregisterManager(EntityManager* manager) {

    componentPools.erase(manager);

}

template <const std::string& cmpName, typename DataType>
std::unique_ptr<ComponentFactory> Component<cmpName, DataType>::factory;

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::registerComponent() {
    if (!factory)
        componentUtils::factoryMap.emplace(cmpName, std::unique_ptr<ComponentFactory>(new ComponentFactoryInternal()));
}

template <const std::string& cmpName, typename DataType>
std::map<EntityManager*, std::shared_ptr<std::deque<Component<cmpName, DataType>>>> Component<cmpName, DataType>::componentPools{};

template <const std::string& cmpName, typename DataType>
const std::string Component<cmpName, DataType>::getName(){ return cmpName; };

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::build(sol::object& obj) {

        data = buildFromLua<DataType>(obj);

}

void registerAllComponents();

//  deallocates memory for factories. Mainly for end of runtime cleanup
void deregisterAllComponents();

#endif // COMPONENT_H_INCLUDED
