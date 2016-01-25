#include "component.h"
#include "renderable.h"
#include "scriptcomponent.h"
#include <cstdlib>
#include <string>

ComponentBase::ComponentBase() {}
ComponentBase::~ComponentBase() {}

ComponentFactory::ComponentFactory() {};
ComponentFactory::~ComponentFactory() {};

std::map<std::string, ComponentFactory*> componentUtils::factoryMap;

void deregisterAllComponents() {
    using namespace componentUtils;
    std::vector<ComponentFactory*> factories;
    for (auto factoryPair : factoryMap) {
        factories.push_back(factoryPair.second);

    }
    factoryMap.clear();
    for (auto factory : factories) {
        delete factory;
        factory = nullptr;

    }
}

template<>
int32_t buildFromString<int32_t>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    return std::atoi(str[pos].c_str());
}

template<>
int64_t buildFromString<int64_t>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    return std::strtol(str[pos].c_str(), nullptr, 0);
}

template<>
uint32_t buildFromString<uint32_t>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {;
    return std::strtoul(str[pos].c_str(), nullptr, 0);
}

template<>
uint64_t buildFromString<uint64_t>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    return std::strtoul(str[pos].c_str(), nullptr, 0);
}

template<>
float buildFromString<float>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    return std::strtof(str[pos].c_str(), nullptr);
}

template<>
std::string buildFromString<std::string>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    return str[pos];
}
