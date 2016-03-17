#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#include <cstdint>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <iostream>
#include "componentName.h"

class EntityManager;

class ComponentFactory;

namespace componentUtils {
    extern std::map<std::string, ComponentFactory*> factoryMap;
}

// utility functions to turn parsed strings into objects
template <typename DataType>
DataType buildFromString(std::vector<std::string> str, std::vector<std::string>::size_type pos);

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

    virtual std::size_t build(EntityManager* manager, size_t idx, std::vector<std::string> instructions) = 0;
    virtual std::size_t build(EntityManager* manager, size_t idx, ComponentBase* cmp) = 0;
    virtual std::size_t build(EntityManager* manager, std::vector<std::string> instructions) = 0;
    virtual std::size_t build(EntityManager* manager, ComponentBase* cmp) = 0;

    virtual std::vector<std::string> tokenize(std::string instructions) = 0;

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

        std::size_t build(EntityManager* manager, size_t idx, std::vector<std::string> instructions);

        std::size_t build(EntityManager* manager, size_t idx, ComponentBase* cmp);

        std::size_t build(EntityManager* manager, std::vector<std::string> instructions);

        std::size_t build(EntityManager* manager, ComponentBase* cmp);

        void registerManager(EntityManager*);

        void deregisterManager(EntityManager*);

        std::vector<std::string> tokenize(std::string instructions);
    };

    typedef std::deque<Component<cmpName, DataType>> cmpPool;
public:
    static std::map<EntityManager*, cmpPool> componentPools;
    static const std::string getName();
    static ComponentFactoryInternal* factory;
    static size_t reserveCapacity;

    //creates a ComponentFactory for the component type
    static void registerComponent(size_t capacity);

    void build(std::vector<std::string> instructions);

    Component() : data{} {}
    Component(const Component& other) : data(other.data) {}
    Component(const DataType& newData) : data(newData) {}
    Component(Component&& other) : data(other.data) {}
    Component(DataType&& newData) : data(newData) {}
    Component(std::vector<std::string> instructions) : data{} {
        build(instructions);
    }

    Component& operator=(Component other) { data = other.data; return *this; }

    DataType data;

};

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, size_t idx, std::vector<std::string> instructions) {
    auto& pool = Component<cmpName, DataType>::componentPools[manager];
    pool[idx].build(instructions);
    return idx;

}

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, size_t idx, ComponentBase* cmp) {
    auto& pool = Component<cmpName, DataType>::componentPools[manager];
    pool[idx] = *static_cast<Component<cmpName, DataType>*>(cmp);
    return idx;
}

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, std::vector<std::string> instructions) {
    auto& pool = Component<cmpName, DataType>::componentPools[manager];
    pool.emplace_back(instructions);
    return pool.size() - 1;
}

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, ComponentBase* cmp) {
    auto& pool = Component<cmpName, DataType>::componentPools[manager];
    pool.emplace_back(*static_cast<Component<cmpName, DataType>*>(cmp));
    return pool.size() - 1;
}

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::ComponentFactoryInternal::registerManager(EntityManager* manager) {

    Component<cmpName, DataType>::componentPools[manager];

}

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::ComponentFactoryInternal::deregisterManager(EntityManager* manager) {

    Component<cmpName, DataType>::componentPools.erase(manager);

}

//splits component definition into a vector that consists of
/*
    component
    {Name of Component}
    data that makes up component
*/
template <const std::string& cmpName, typename DataType>
std::vector<std::string> Component<cmpName, DataType>::ComponentFactoryInternal::tokenize(std::string instructions) {
    auto beginCmp = instructions.find_first_of(" \n\f\r\t\v", instructions.find("component:" + cmpName));
    std::vector<std::string> tokenizedString;
    tokenizedString.reserve(5);
    std::string::size_type tokenStart = std::string::npos;
    std::locale loc;
    uint32_t nesting = 0;
    std::string::size_type nestStart = std::string::npos;
    if (beginCmp != std::string::npos) {
        tokenizedString.push_back("component");
        tokenizedString.push_back(cmpName);
        for(auto start = beginCmp; start != std::string::npos &&
                start < instructions.size(); ++start) {
            auto ch = instructions[start];
            if (ch == '<') {
                if (nesting == 0) {
                    nestStart = start + 1;
                    if (tokenStart != std::string::npos)
                        tokenizedString.emplace_back(instructions.substr(tokenStart, start - tokenStart));
                }
                ++nesting;
            }
            else if (ch == '>' && nesting != 0) {
                --nesting;
                if (nesting == 0) {
                    tokenizedString.emplace_back(instructions.substr(nestStart, start - nestStart));
                    nestStart = std::string::npos;
                    continue;
                }
            }

            if (nesting == 0) {
                if (!std::isspace(ch, loc) && ch != ',' && ch != '\n' && tokenStart == std::string::npos) {
                    tokenStart = start;
                } else if ( (std::isspace(ch, loc) || ch == ',' || ch== '\n') && tokenStart != std::string::npos) {
                    tokenizedString.emplace_back(instructions.substr(tokenStart, start - tokenStart));
                    tokenStart = std::string::npos;
                }
            }

        }

        if (tokenStart != std::string::npos)
            tokenizedString.emplace_back(instructions.substr(tokenStart));

    }

    return tokenizedString;

}

template <const std::string& cmpName, typename DataType>
typename Component<cmpName, DataType>::ComponentFactoryInternal* Component<cmpName, DataType>::factory{nullptr};

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::registerComponent(size_t capacity) {
    if (!factory)
        factory = new ComponentFactoryInternal();
    componentUtils::factoryMap[cmpName] = factory;
    reserveCapacity = capacity;
}

template <const std::string& cmpName, typename DataType>
std::map<EntityManager*, std::deque<Component<cmpName, DataType>>> Component<cmpName, DataType>::componentPools{};

template <const std::string& cmpName, typename DataType>
size_t Component<cmpName, DataType>::reserveCapacity{1};

template <const std::string& cmpName, typename DataType>
const std::string Component<cmpName, DataType>::getName(){ return cmpName; };

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::build(std::vector<std::string> instructions) {

    if (instructions.size() >= 3) {
            data = buildFromString<DataType>(instructions, 2);
    }

}

void registerAllComponents();

//  deallocates memory for factories. Mainly for end of runtime cleanup
void deregisterAllComponents();

#endif // COMPONENT_H_INCLUDED
