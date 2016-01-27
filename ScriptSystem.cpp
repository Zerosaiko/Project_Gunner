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
            if (script != entity->end()) {
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
                int beg = -1, end = 0;
                for(size_t i = 0; i < scr.tokenizedScript.size(); ++i) {
                    if (scr.tokenizedScript[i] == "@start") beg = i;
                    if (scr.tokenizedScript[i][0] == '@') end = i;
                }
                if (end == beg) end = scr.tokenizedScript.size();
                if (beg >= 0 && beg < scr.tokenizedScript.size())
                    execute(scr, id, beg, end);


            }
        }
    }
}

void ScriptSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
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
            int beg = -1, end = 0;
            for(size_t i = 0; i < script.tokenizedScript.size() && script.updateRate > 0; ++i) {
                //std::cout << i << " - " << script.tokenizedScript[i] << "-\n";
                if (script.tokenizedScript[i] == "@onUpdate") beg = i;
                if (script.tokenizedScript[i][0] == '@') end = i;
            }
            if (beg == -1) beg = script.tokenizedScript.size();
            if (end == beg) end = script.tokenizedScript.size();
            if (beg >= 0 && beg < script.tokenizedScript.size() )
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
            if (script.tokenizedScript[++i] == "%parent") {
                targetID = manager->getParent(id);
                ++i;
            }
            else {
                children = script.tokenizedScript[i] == "%children";
                i += children;
            }
            while(i < end && script.tokenizedScript[i] != "stop_") {
                (str += "\n") += script.tokenizedScript[i++];
            }
            if (!children) {
                manager->addComponent(str, targetID);
            }
            else {
                for (auto& child: manager->getChildren(id)) {
                    manager->addComponent(str, child);
                }
            }

        } else if (script.tokenizedScript[i] == "end_script" ) {
            std::cout << "ending script " << id << '\n';
            manager->removeComponent<Component<Script::name, Script>>(id);
            ++i;
        } else if (script.tokenizedScript[i] == "destroy" ) {
            uint32_t targetID = id;
            bool children = false;
            if (script.tokenizedScript[++i] == "%parent") {
                targetID = manager->getParent(id);
                ++i;
            }
            else {
                children = script.tokenizedScript[i] == "%children";
                i += children;
            }
            if (!children)
                manager->entitiesToDestroy.insert(targetID);
            else {
                for (auto& child: manager->getChildren(id)) {
                    manager->entitiesToDestroy.insert(child);
                }
            }
        } else ++i;
    }
}
