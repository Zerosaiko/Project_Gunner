#ifndef SPAWNSYSTEM_H_INCLUDED
#define SPAWNSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include <unordered_map>
#include "Spawner.h"
#include "displace.h"

class SpawnSystem : public EntitySystem {

public:
    SpawnSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<std::pair<EntityManager::ComponentHandle const *,EntityManager::ComponentHandle const *>> entities;

    std::vector<Component<Position::name, Position>>* positionPool;

    std::vector<Component<Spawner::name, Spawner>>* spawnPool;

    void defaultPositioning(Spawner& spawner, Position& position);

};


#endif // SPAWNSYSTEM_H_INCLUDED
