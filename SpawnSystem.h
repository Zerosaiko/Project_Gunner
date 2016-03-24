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

    struct EntityPreallocationInfo {

        size_t dataIdx;

        size_t currentSpawnCount;

        std::vector<uint32_t> idList;

        std::vector< std::vector< std::vector< std::string > > > data;

        EntityPreallocationInfo( size_t spawnCnt);

    };

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<std::pair<EntityManager::ComponentHandle const *,EntityManager::ComponentHandle const *>> entities;

    std::deque<Component<Position::name, Position>>* positionPool;

    std::deque<Component<Spawner::name, Spawner>>* spawnPool;

    std::deque<EntityPreallocationInfo> preAllocationData;

    size_t preAlloIdx;

    size_t totalSpawnCount;

};


#endif // SPAWNSYSTEM_H_INCLUDED
