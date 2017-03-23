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
#include "Transform.h"
#include "health.h"
#include "shieldComponent.h"
#include "lifeTimer.h"
#include <cstdlib>
#include <string>

ComponentBase::ComponentBase() {}
ComponentBase::~ComponentBase() {}

ComponentFactory::ComponentFactory() {};
ComponentFactory::~ComponentFactory() {};

void registerAllComponents() {

    Component<Sprite::name, Sprite>::registerComponent();
    Component<Position::name, Position>::registerComponent();
    Component<Velocity::name, Velocity>::registerComponent();
    Component<PlayerCmp::name, PlayerCmp>::registerComponent();
    Component<Bounds::name, Bounds>::registerComponent();
    Component<Collider::name, Collider>::registerComponent();
    Component<Spawner::name, Spawner>::registerComponent();
    Component<delayComponent::fullDelay, float>::registerComponent();
    Component<delayComponent::pauseDelay, float>::registerComponent();
    Component<Transform::name, Transform>::registerComponent();
    Component<Animation::name, Animation>::registerComponent();
    Component<Health::name, Health>::registerComponent();
    Component<HealthRegen::name, HealthRegen>::registerComponent();
    Component<Shield::name, Shield>::registerComponent();
    Component<lifeTimerName, float>::registerComponent();

}

std::map<std::string, std::unique_ptr<ComponentFactory>> componentUtils::factoryMap;

void deregisterAllComponents() {
    using namespace componentUtils;
    for (auto it = factoryMap.begin(); it != factoryMap.end(); it = factoryMap.erase(it));
    /*
    std::vector<ComponentFactory*> factories;
    for (auto factoryPair : factoryMap) {
        factories.push_back(factoryPair.second);

    }
    factoryMap.clear();
    for (auto& factory : factories) {
        delete factory;
        factory = nullptr;

    }*/
}

template<>
int32_t buildFromLua<int32_t>(sol::object& cmp) {
    return cmp.as<int32_t>();
}

template<>
int64_t buildFromLua<int64_t>(sol::object& cmp) {
    return cmp.as<int64_t>();
}

template<>
uint32_t buildFromLua<uint32_t>(sol::object& cmp) {
    return cmp.as<uint32_t>();
}

template<>
uint64_t buildFromLua<uint64_t>(sol::object& cmp) {
    return cmp.as<uint64_t>();
}

template<>
float buildFromLua<float>(sol::object& cmp) {
    return cmp.as<float>();
}

template<>
std::string buildFromLua<std::string>(sol::object& cmp) {
    return cmp.as<std::string>();
}
