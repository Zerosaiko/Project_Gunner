#include "TagManager.h"

void TagManager::tagEntity(std::string tag, uint32_t entityID) {

    entityIDs[tag] = entityID;

}

void TagManager::untagEntity(std::string tag, uint32_t entityID) {

    auto it = entityIDs.find(tag);
    if (it != entityIDs.end() && it->second == entityID)
        entityIDs.erase(it);

}

void TagManager::untagEntity(uint32_t entityID) {

    for( auto it = entityIDs.begin(); it != entityIDs.end(); ++it) {
        if (it->second == entityID)
            entityIDs.erase(it);
    };

}

void TagManager::removeTag(std::string tag) {
    entityIDs.erase(tag);
}

uint32_t* TagManager::getIDByTag(std::string tag) {

    auto it = entityIDs.find(tag);
    uint32_t* id = nullptr;
    if (it != entityIDs.end())
        id = &it->second;
    return id;

}
