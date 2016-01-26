#ifndef TAGMANAGER_H_INCLUDED
#define TAGMANAGER_H_INCLUDED

#include <unordered_map>
#include <string>
#include <cstdint>

class EntityManager;

//  uniquely maps an ID to a string
class TagManager {
private:

    std::unordered_map<std::string, uint32_t > entityIDs;

    EntityManager* manager;

public:

    TagManager(EntityManager* manager);

    void tagEntity(std::string tag, uint32_t entityID);

    void untagEntity(std::string tag, uint32_t entityID);

    void untagEntity(uint32_t entityID);

    void removeTag(std::string tag);

    uint32_t* getIDByTag(std::string tag);

};

#endif // TAGMANAGER_H_INCLUDED
