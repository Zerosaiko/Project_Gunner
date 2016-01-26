#ifndef GROUPMANAGER_H_INCLUDED
#define GROUPMANAGER_H_INCLUDED

#include <unordered_map>
#include <string>
#include <cstdint>
#include <vector>

class EntityManager;

//  class that groups various entities under a common identifier and allows retrieval of IDs by group
class GroupManager {
private:

    std::unordered_map<std::string, std::vector<uint32_t> > entityIDs;

    EntityManager* manager;

public:

    GroupManager(EntityManager* manager);

    void groupEntity(std::string group, uint32_t entityID);

    void ungroupEntity(std::string group, uint32_t entityID);

    void ungroupEntity(uint32_t entityID);

    void removeGroup(std::string group);

    std::vector<uint32_t> const* getIDGroup(std::string group);

};


#endif // GROUPMANAGER_H_INCLUDED
