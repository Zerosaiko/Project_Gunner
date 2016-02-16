#include "component.h"
#include "renderable.h"
#include "displace.h"
#include "scriptcomponent.h"
#include "playerComponents.h"
#include "boundsComponent.h"
#include "collider.h"
#include "Spawner.h"
#include "delayComponent.h"
#include <cstdlib>
#include <string>

ComponentBase::ComponentBase() {}
ComponentBase::~ComponentBase() {}

ComponentFactory::ComponentFactory() {};
ComponentFactory::~ComponentFactory() {};

void registerAllComponents() {

    Component<Renderable::name, Renderable>::registerComponent(1024);
    Component<Script::name, Script>::registerComponent(128);
    Component<Position::name, Position>::registerComponent(1024);
    Component<Velocity::name, Velocity>::registerComponent(1024);
    Component<PlayerCmp::speed, float>::registerComponent(16);
    Component<PlayerCmp::focusSpeed, float>::registerComponent(16);
    Component<Bounds::name, Bounds>::registerComponent(1024);
    Component<Collider::name, Collider>::registerComponent(1024);
    Component<Spawner::name, Spawner>::registerComponent(256);
    Component<delayComponent::fullDelay, float>::registerComponent(256);
    Component<delayComponent::pauseDelay, float>::registerComponent(256);

}

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
