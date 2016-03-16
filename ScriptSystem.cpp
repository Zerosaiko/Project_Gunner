#include "ScriptSystem.h"
#include "SDL.h"

ScriptSystem::ScriptSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    scriptPool = manager->getComponentPool<Component<Script::name, Script>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
};

void ScriptSystem::initialize() {}

void ScriptSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto script = entity->find("script");
        if (script != entity->end() && script->second.active) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.push_back(&script->second);

            Script& scr = (*scriptPool)[script->second.index].data;
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

void ScriptSystem::removeEntity(uint32_t id) {
    if (!hasEntity[id]) return;

    const auto& entity = entities[id];
    Script& scr = (*scriptPool)[entity->index].data;
    size_t beg = scr.tokenizedScript.size(), end = 0;
    for(size_t i = 0; i < scr.tokenizedScript.size(); ++i) {
        if (scr.tokenizedScript[i] == "@onEnd") beg = i;
        if (scr.tokenizedScript[i][0] == '@') end = i;
    }
    if (end == beg) end = scr.tokenizedScript.size();
    if (beg < scr.tokenizedScript.size())
        execute(scr, id, beg, end);
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void ScriptSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!entity->active) {
        removeEntity(id);
    } else {
        const auto& fullEntity = manager->getEntity(id);
        auto delay = fullEntity->find("fullDelay");
        auto pause = fullEntity->find("pauseDelay");
        if ( (delay != fullEntity->end() && delay->second.active) || (pause != fullEntity->end() && pause->second.active) ) {
            removeEntity(id);
        } else if (entity->dirty) {
            Script& scr = (*scriptPool)[entity->index].data;
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

void ScriptSystem::process(float dt) {
    auto startT = SDL_GetPerformanceCounter();
    dt *= 1000;
    for(size_t idx = 0; idx < entities.size(); ++idx) {
        const auto& entity = entities[idx];
        Script& script = (*scriptPool)[entity->index].data;
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
                    execute(script, idxToID[idx], beg, end);
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
                manager->destroyEntity(targetID);
            }
            else if (children){
                for (auto& child: manager->getChildren(id)) {
                    manager->destroyEntity(child);
                }
            }
            else if (!group.empty()) {
                auto groupVec = manager->groupManager.getIDGroup(group);
                if (groupVec) {
                    for (auto& entity: *groupVec) {
                        manager->destroyEntity(entity);
                    }
                }
            }
        } else ++i;
    }
}
