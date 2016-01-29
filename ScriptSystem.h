#ifndef SCRIPTSYSTEM_H_INCLUDED
#define SCRIPTSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include <unordered_map>
#include "scriptcomponent.h"

class ScriptSystem : public EntitySystem {

public:
    ScriptSystem(EntityManager& manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

    void execute(Script& scr, uint32_t id, std::vector<std::string>::size_type beg, std::vector<std::string>::size_type end );

private:

    std::unordered_map<uint32_t, std::vector<EntityManager::component_pair const *>::size_type> entityIDs;

    std::vector<std::vector<EntityManager::component_pair const *>::size_type> freeIDXs;

    std::vector<EntityManager::component_pair const *> entities;

    std::vector<Component<Script::name, Script>>* scriptPool;

};

#endif // SCRIPTSYSTEM_H_INCLUDED
