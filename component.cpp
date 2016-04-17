#include "component.h"
#include "renderable.h"
#include "displace.h"
#include "scriptcomponent.h"
#include "playerComponents.h"
#include "boundsComponent.h"
#include "animation.h"
#include "collider.h"
#include "Spawner.h"
#include "delayComponent.h"
#include "Orientation.h"
#include "health.h"
#include "shieldComponent.h"
#include <cstdlib>
#include <string>

ComponentBase::ComponentBase() {}
ComponentBase::~ComponentBase() {}

ComponentFactory::ComponentFactory() {};
ComponentFactory::~ComponentFactory() {};

void registerAllComponents() {

    Component<Sprite::name, Sprite>::registerComponent();
    Component<Script::name, Script>::registerComponent();
    Component<Position::name, Position>::registerComponent();
    Component<Velocity::name, Velocity>::registerComponent();
    Component<PlayerCmp::name, PlayerCmp>::registerComponent();
    Component<Bounds::name, Bounds>::registerComponent();
    Component<Collider::name, Collider>::registerComponent();
    Component<Spawner::name, Spawner>::registerComponent();
    Component<delayComponent::fullDelay, float>::registerComponent();
    Component<delayComponent::pauseDelay, float>::registerComponent();
    Component<Orientation::name, Orientation>::registerComponent();
    Component<Animation::name, Animation>::registerComponent();
    Component<Health::name, Health>::registerComponent();
    Component<HealthRegen::name, HealthRegen>::registerComponent();
    Component<Shield::name, Shield>::registerComponent();

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
int32_t buildFromString<int32_t>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    return std::atoi(str[pos++].c_str());
}

template<>
int64_t buildFromString<int64_t>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    return std::strtol(str[pos++].c_str(), nullptr, 0);
}

template<>
uint32_t buildFromString<uint32_t>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {;
    return std::strtoul(str[pos++].c_str(), nullptr, 0);
}

template<>
uint64_t buildFromString<uint64_t>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    return std::strtoul(str[pos++].c_str(), nullptr, 0);
}

template<>
float buildFromString<float>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    return std::strtof(str[pos++].c_str(), nullptr);
}

template<>
std::string buildFromString<std::string>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    return str[pos++];
}
