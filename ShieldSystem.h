#ifndef SHIELDSYSTEM_H_INCLUDED
#define SHIELDSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include "shieldComponent.h"
#include "Message.h"

class ShieldSystem : public EntitySystem {

public:
    ShieldSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

    bool shieldHit(Message& message);

private:
    std::unordered_map<uint32_t, EntityManager::ComponentHandle const *> entities;

    std::weak_ptr<std::deque<Component<Shield::name, Shield>>> shieldPool;

    std::function<bool(Message& message)> onShieldHit;

};


#endif // SHIELDSYSTEM_H_INCLUDED
