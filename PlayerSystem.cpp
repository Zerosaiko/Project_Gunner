#include "PlayerSystem.h"
#include "displace.h"
#include "Spawner.h"
#include "collider.h"
#include "Transform.h"
#include "renderable.h"
#include "delayComponent.h"
#include "lifeTimer.h"
#include "shieldComponent.h"

PlayerSystem::PlayerSystem(EntityManager* const manager, int32_t priority, sol::state& state) : EntitySystem{manager, priority},
        luaState(state) {
    playerPool = manager->getComponentPool<Component<PlayerCmp::name, PlayerCmp>>();
    randEngine.seed(time(nullptr));
    luaState.set_function("playerHit", &PlayerSystem::playerHit, this);
}

void PlayerSystem::initialize() {

}

void PlayerSystem::addEntity(uint32_t id) {
    auto entity = manager->getEntity(id);
    if (entity) {
        const auto &components = entity->components;
        auto playerCmp = components.find("player");
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay == components.end() || !delay->second.active) && (pause == components.end() || !pause->second.active)
            && playerCmp != components.end() && playerCmp->second.active) {

            entities.emplace(id, &playerCmp->second);
            manager->groupManager.groupEntity("player", id);


        }
    }

}

void PlayerSystem::removeEntity(uint32_t id) {
    entities.erase(id);
    manager->groupManager.ungroupEntity("player", id);
}

void PlayerSystem::refreshEntity(uint32_t id) {
    auto entity = entities.find(id);
    if (entity != entities.end() && !entity->second->active) {
        removeEntity(id);
    } else if (entity != entities.end()) {
        auto fullEntity = manager->getEntity(id);
        const auto &components = fullEntity->components;
        auto delay = components.find("fullDelay");
        auto pause = components.find("pauseDelay");
        if ( (delay != components.end() && delay->second.active) || (pause != components.end() && pause->second.active) ) {
            removeEntity(id);
        }
    } else {
        addEntity(id);
    }
}

void PlayerSystem::process(float dt) {

    float adjustedDT = dt * 1000.0f;

    auto playerPool = this->playerPool.lock();

    for (auto& entity : entities) {

        PlayerCmp& playerData = playerPool->operator[](entity.second->index).data;
        if (!playerData.alive/* && playerData.lives > 0*/) {
            playerData.deathTimer -= adjustedDT;
            if (playerData.deathTimer <= 0) {
                playerData.deathTimer = 3000.0f;
                playerData.alive = true;
                --playerData.lives;
                Transform playerPos;

                playerPos.local.setTranslate(180, 360);
                playerPos.local.setAngle(90);
                manager->addComponent<Component<Transform::name, Transform>>(playerPos, entity.first);
                manager->addComponent<Component<Shield::name, Shield>>(Shield(2500), entity.first);
                luaState["id"] = entity.first;
                luaState.script(R"(
                    manager:addComponent("collider",
                    {
                        ["group"] = "Player",
                        ["handlers"] = { ['Enemy'] = function (a, b) playerHit(a, b) end, ['EnemyBullet'] = function (a, b) playerHit(a, b) end },
                        ["collider"] = {["type"] = "Circle",
                        ["x"] = 0,
                        ["y"] = 0,
                        ["radius"] = 3}
                    }, id)
                )");
                luaState["id"] = sol::nil;
                playerData.blackboard["onRevive"]();
            }
        } else {
            if (playerData.currentTime < playerData.cooldown)
                playerData.currentTime += adjustedDT;
            auto velPool = manager->getComponentPool<Component<Velocity::name, Velocity>>().lock();
            auto ent = manager->getEntity(entity.first);
            const auto &components = ent->components;
            auto velCmpHdl = components.find("velocity");
            if (velCmpHdl != components.end()) {
                auto& velCmp = velPool->operator[](velCmpHdl->second.index).data;
                playerData.blackboard["update"](dt, entity.first);
            }

        }

    }

}

bool PlayerSystem::playerHit(uint32_t id1, uint32_t id2) {//
    auto entity = entities.find(id2);
    std::cout << id1 << '\t' << id2 << '\n';
    if (entity != entities.end() && entity->second->active) {

        auto playerPool = this->playerPool.lock();
        PlayerCmp& playerCmp = playerPool->operator[](entity->second->index).data;
        auto mgEnt = manager->getEntity(id2);
        const auto &components = mgEnt->components;
        auto shieldIT = components.find("shield");
        if (playerCmp.alive && (shieldIT == components.end() || !shieldIT->second.active ) && manager->getEntity(id1) != nullptr) {

            std::cout << "Player hit\tHitter: " << id1 << "\tPlayer: " << id2 << '\n';
            manager->destroyEntity(id1);

            auto positionPool = manager->getComponentPool<Component<Transform::name, Transform>>().lock();
            const auto& fullEntity = manager->getEntity(id2);
            const auto &components = fullEntity->components;
            auto posCmp = components.find("transform");
            if (posCmp != components.end()) {

                std::uniform_real_distribution<float> angleDist{0.0f, 359.99999f};
                std::uniform_real_distribution<float> scaleDist{0.5f, 1.0f};
                std::uniform_real_distribution<float> scaleFacDist{0.6f, 1.10f};
                std::uniform_real_distribution<float> coordDist{-4, 4};
                std::uniform_real_distribution<float> animSpdDist{0.75f, 2.0f};
                std::uniform_int_distribution<int16_t> zOrderDist{30, 45};
                    luaState["explosionAnim"] = luaState.create_table_with(
                        "frames", luaState.create_table_with(1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8)
                    );

                Transform& pos = positionPool->operator[](posCmp->second.index).data;
                float x = 0.0f, y = 0.0f;
                std::tie(x, y) = pos.worldPresent.getPos();
                luaState["delay"] = 0.0f;
                float ceiling = 0.0f;
                float floor = 0.0f;
                float delay = 0.0f;
                for (uint8_t i = 0; i < 12; ++i) {
                    float xScale = scaleDist(randEngine), yScale = xScale * scaleFacDist(randEngine);
                    float animSpd = animSpdDist(randEngine) / xScale;
                    luaState["explosionAnim"]["frameLengths"] =
                        luaState.create_table_with(
                            1, 250 / animSpd, 2, 180 / animSpd,
                            3, 220 / animSpd, 4, 200 / animSpd,
                            5, 300 / animSpd, 6, 400 / animSpd,
                            7,  200 / animSpd, 8, 1);
                    std::uniform_real_distribution<float> delayDist{0.0f, ceiling += 15.0f};
                    delay += delayDist(randEngine);
                    luaState["delay"] = delay;
                    luaState["explId"] = manager->createEntity();
                    luaState["orient"] = luaState.create_table_with(
                        "transform", luaState.create_table_with(
                            "translate", luaState.create_table_with(
                                "x", x + coordDist(randEngine),
                                "y", y + coordDist(randEngine)
                            ),
                            "angle", angleDist(randEngine),
                            "scale", luaState.create_table_with("x", xScale, "y", yScale)
                        )

                    );
                    luaState.script(R"(
                        manager:addComponent("transform", orient, explId)
                        manager:addComponent("animation", explosionAnim, explId)

                    )");
                    luaState["zOrder"] = zOrderDist(randEngine);
                    luaState.script(R"(
                        manager:addComponent("sprite", { ["fileName"] = "possibleExplosions", ["spritePos"] = 0, ["zOrder"] = zOrder}, explId)
                        manager:addComponent("fullDelay", delay, explId)
                        manager:addComponent("lifeTimer", 6000, explId)
                        zOrder = nil
                        delay = nil
                    )");

                }

                luaState.script(R"(
                        explId = nil
                        explosionAnim = nil
                        orient = nil)");

            }

            manager->removeComponent<Component<Transform::name, Transform>>( id2);
            manager->removeComponent<Component<Collider::name, Collider>>( id2);
            playerCmp.alive = false;
            playerCmp.blackboard["onDeath"]();
            return true;
        }

    }
    return false;
}
