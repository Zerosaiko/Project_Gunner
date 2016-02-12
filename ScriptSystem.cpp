#include "ScriptSystem.h"
#include "SDL.h"

ScriptSystem::ScriptSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    scriptPool = manager->getComponentPool<Component<Script::name, Script>>();
};

void ScriptSystem::initialize() {}

void ScriptSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto script = entity->find("script");
            if (script != entity->end() && script->second.first) {
                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.push_back(&script->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    entities[idx] = &script->second;
                }

                Script& scr = (*scriptPool)[script->second.second].data;
                size_t beg = scr.tokenizedScript.size(), end = 0;
                for(size_t i = 0; i < scr.tokenizedScript.size(); ++i) {
                    if (scr.tokenizedScript[i] == "@onStart") beg = i;
                    if (scr.tokenizedScript[i][0] == '@') end = i;
                }
                if (end == beg) end = scr.tokenizedScript.size();
                if (beg < scr.tokenizedScript.size())
                    execute(scr, id, beg, end);


            }
        }
    }
}

void ScriptSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        auto& entity = entities[entityID->second];
        Script& scr = (*scriptPool)[entity->second].data;
        size_t beg = scr.tokenizedScript.size(), end = 0;
        for(size_t i = 0; i < scr.tokenizedScript.size(); ++i) {
            if (scr.tokenizedScript[i] == "@onEnd") beg = i;
            if (scr.tokenizedScript[i][0] == '@') end = i;
        }
        if (end == beg) end = scr.tokenizedScript.size();
        if (beg < scr.tokenizedScript.size())
            execute(scr, id, beg, end);
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    }
}

void ScriptSystem::refreshEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !entities[entityID->second]->first) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID == entityIDs.end() ) {
        addEntity(id);
    }

}

void ScriptSystem::process(float dt) {
    auto startT = SDL_GetPerformanceCounter();
    dt *= 1000;
    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Script& script = (*scriptPool)[entity->second].data;
        if (script.updateRate > 0) {
            size_t beg = script.tokenizedScript.size(), end = 0;
            for(size_t i = 0; i < script.tokenizedScript.size() && script.updateRate > 0; ++i) {
                //std::cout << i << " - " << script.tokenizedScript[i] << "-\n";
                if (script.tokenizedScript[i] == "@onUpdate") beg = i;
                if (script.tokenizedScript[i][0] == '@') end = i;
            }
            if (end == beg) end = script.tokenizedScript.size();
            if (beg < script.tokenizedScript.size() )
                for(script.dt += dt; script.dt > script.updateRate; script.dt -= script.updateRate) {
                    execute(script, entityID.first, beg, end);
                }
        }
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

void ScriptSystem::execute(Script& script, uint32_t id, std::vector<std::string>::size_type beg, std::vector<std::string>::size_type end ) {
    std::string str;
    for(auto i = beg; i < end;) {
        if (script.tokenizedScript[i] == "create") {
            str.clear();
            uint32_t targetID = id;
            bool children = false;
            std::string group;
            int8_t nesting = 1;
            if (script.tokenizedScript[++i] == "%new") {
                targetID = manager->createEntity();
                ++i;
            }
            else if (script.tokenizedScript[i] == "%parent") {
                targetID = manager->getParent(id);
                ++i;
            }
            else if (script.tokenizedScript[i] == "%tag"){
                auto entity = manager->tagManager.getIDByTag(script.tokenizedScript[++i]);
                if (entity) targetID = *entity;
                ++i;
            }
            else if (script.tokenizedScript[i] == "%group") {
                group = script.tokenizedScript[++i];
                ++i;
            }
            else {
                children = script.tokenizedScript[i] == "%children";
                i += children;
            }
            while(i < end && nesting > 0) {
                if (script.tokenizedScript[i] == "create") ++nesting;
                else if (script.tokenizedScript[i] == "stop_") --nesting;
                (str += "\n") += script.tokenizedScript[i++];
            }
            if (group.empty() || !children) {
                manager->addComponent(str, targetID);
            } else if (children) {
                for (auto& child: manager->getChildren(id)) {
                    manager->addComponent(str, child);
                }
            } else if (!group.empty()) {
                auto groupVec = manager->groupManager.getIDGroup(group);
                if (groupVec) {
                    for (auto& entity: *groupVec) {
                        manager->addComponent(str, entity);
                    }
                }
            }

        } else if (script.tokenizedScript[i] == "end_script" ) {
            manager->removeComponent<Component<Script::name, Script>>(id);
            ++i;
        } else if (script.tokenizedScript[i] == "remove") {
            uint32_t targetID = id;
            bool children = false;
            std::string group;
            if (script.tokenizedScript[++i] == "%parent") {
                targetID = manager->getParent(id);
                ++i;
            }
            else if (script.tokenizedScript[i] == "%tag"){
                auto entity = manager->tagManager.getIDByTag(script.tokenizedScript[++i]);
                if (entity) targetID = *entity;
                ++i;
            }
            else if (script.tokenizedScript[i] == "%group") {
                group = script.tokenizedScript[++i];
                ++i;
            }
            else {
                children = script.tokenizedScript[i] == "%children";
                i += children;
            }
            if (group.empty() || !children) {
                manager->removeComponent(script.tokenizedScript[i], targetID);
            }
            else if (children){
                for (auto& child: manager->getChildren(id)) {
                    manager->removeComponent(script.tokenizedScript[i],  child);
                }
            }
            else if (!group.empty()) {
                auto groupVec = manager->groupManager.getIDGroup(group);
                if (groupVec) {
                    for (auto& entity: *groupVec) {
                        manager->removeComponent(script.tokenizedScript[i],  entity);
                    }
                }
            }

        } else if (script.tokenizedScript[i] == "destroy" ) {
            uint32_t targetID = id;
            bool children = false;
            std::string group;
            if (script.tokenizedScript[++i] == "%parent") {
                targetID = manager->getParent(id);
                ++i;
            }
            else if (script.tokenizedScript[i] == "%tag"){
                auto entity = manager->tagManager.getIDByTag(script.tokenizedScript[++i]);
                if (entity) targetID = *entity;
                ++i;
            }
            else if (script.tokenizedScript[i] == "%group") {
                group = script.tokenizedScript[++i];
                ++i;
            }
            else {
                children = script.tokenizedScript[i] == "%children";
                i += children;
            }

            if (group.empty() || !children) {
                manager->entitiesToDestroy.insert(targetID);
            }
            else if (children){
                for (auto& child: manager->getChildren(id)) {
                    manager->entitiesToDestroy.insert(child);
                }
            }
            else if (!group.empty()) {
                auto groupVec = manager->groupManager.getIDGroup(group);
                if (groupVec) {
                    for (auto& entity: *groupVec) {
                        manager->entitiesToDestroy.insert(entity);
                    }
                }
            }
        } else ++i;
    }
}
