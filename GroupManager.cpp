#include "GroupManager.h"
#include "EntityManager.h"
#include <algorithm>

GroupManager::GroupManager(EntityManager* manager) : manager(manager) {}

void GroupManager::groupEntity(std::string group, uint32_t entityID) {

    entityIDs[group].push_back(entityID);
    manager->entitiesToRefresh.insert(entityID);

}

void GroupManager::ungroupEntity(std::string group, uint32_t entityID) {

    auto it = entityIDs.find(group);
    if (it != entityIDs.end()) {
        auto itID = std::find(it->second.begin(), it->second.end(), entityID);
        if (itID != it->second.end()) {
            *itID = it->second.back();
            it->second.pop_back();
        }
    }
    manager->entitiesToRefresh.insert(entityID);


}

void GroupManager::ungroupEntity(uint32_t entityID) {

    for( auto it = entityIDs.begin(); it != entityIDs.end(); ++it) {
        ungroupEntity(it->first, entityID);
    };
    manager->entitiesToRefresh.insert(entityID);

}

void GroupManager::removeGroup(std::string group) {
    for (auto id : entityIDs[group]) {
        manager->entitiesToRefresh.insert(id);
    }
    entityIDs.erase(group);
}

std::vector<uint32_t> const* GroupManager::getIDGroup(std::string group) {

    auto it = entityIDs.find(group);
    std::vector<uint32_t> const * idGroup = nullptr;
    if (it != entityIDs.end())
        idGroup = &it->second;
    return idGroup;

}
