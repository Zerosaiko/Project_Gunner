#include "TagManager.h"
#include "EntityManager.h"

TagManager::TagManager(EntityManager* manager) : manager{manager} {}

void TagManager::tagEntity(std::string tag, uint32_t entityID) {

    entityIDs[tag] = entityID;
    manager->entitiesToRefresh.insert(entityID);

}

void TagManager::untagEntity(std::string tag, uint32_t entityID) {

    auto it = entityIDs.find(tag);
    if (it != entityIDs.end() && it->second == entityID)
        entityIDs.erase(it);
    manager->entitiesToRefresh.insert(entityID);

}

void TagManager::untagEntity(uint32_t entityID) {
    for( auto it = entityIDs.begin(); it != entityIDs.end();) {
        if (it->second == entityID)
            it = entityIDs.erase(it);
        else ++it;
    }
    manager->entitiesToRefresh.insert(entityID);

}

void TagManager::removeTag(std::string tag) {
    uint32_t id = entityIDs[tag];
    entityIDs.erase(tag);
    manager->entitiesToRefresh.insert(id);
}

uint32_t* TagManager::getIDByTag(std::string tag) {

    auto it = entityIDs.find(tag);
    uint32_t* id = nullptr;
    if (it != entityIDs.end())
        id = &it->second;
    return id;

}
