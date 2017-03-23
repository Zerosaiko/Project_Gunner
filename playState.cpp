#include "playState.h"
#include "SDL.h"
#include "scriptcomponent.h"
#include <iostream>
#include "sol.hpp"
#include <experimental/optional>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 6000, tfGraph}, renderSys(&manager, 10000, window),
        mInpSys(&manager, 3), boundsSys(&manager, 7500, tfGraph), colSys{&manager, 0, luaState}, spawnSys(&manager, 2),
        tfSyncSys(&manager, 5500, tfGraph), tfCalcSys(&manager, 7250, tfGraph), delaySys(&manager, 1), pauseSys(&manager, 1), playSys{&manager, 0, luaState},
        animSys(&manager, 8500), shieldSys(&manager, 9000), lifeTimerSys(&manager, 1) {

    luaState.open_libraries(sol::lib::base);

    luaState.script("manager = {}");

    luaState["manager"] = &manager;
    luaState.new_usertype<EntityManager>("EntityManager",
        "createEntity", &EntityManager::createEntity,
        "destroyEntity", &EntityManager::destroyEntity,
        "addComponent", static_cast<void(EntityManager::*)(std::string, sol::object, uint32_t)>(&EntityManager::addComponent),
        "removeComponent", static_cast<void(EntityManager::*)(std::string&&, uint32_t)> (&EntityManager::removeComponent),
        "getEntity", &EntityManager::getEntity,
        "noRefresh", &EntityManager::excludeFromRefresh,
        "forceRefresh", &EntityManager::forceRefresh
    );
    luaState["input"] = &input;
    luaState.new_usertype<Input>("Input",
        "keyReleased", &Input::keyReleased,
        "keyHeld", &Input::keyHeld,
        "keyPressed", &Input::keyPressed,
        "keyDown", &Input::keyDown,
        "scancode", &Input::scancode,
        "scancodeSDL", &Input::scancodeSDL
    );
    luaState.new_usertype<Velocity>("Velocity",
        "velX", &Velocity::velX,
        "velY", &Velocity::velY
    );
    luaState.new_usertype<PlayerCmp>("Player",
        "speed", &PlayerCmp::speed,
        "focusSpeed", &PlayerCmp::focusSpeed,
        "alive", &PlayerCmp::alive,
        "lives", &PlayerCmp::lives,
        "aggro", &PlayerCmp::aggro,
        "cooldown", &PlayerCmp::cooldown,
        "currentTime", &PlayerCmp::currentTime
    );
    luaState.new_usertype<EntityManager::EntityInfo>("EntityInfo",
        "getComponent", &EntityManager::EntityInfo::getComponent);

    luaState.script("id = manager:createEntity()");
    luaState.script(R"( manager:addComponent("velocity", {}, id))");
    luaState.script(R"( manager:addComponent("shield", 500, id))");
    luaState.script(R"( manager:addComponent("sprite", { ["fileName"] = "NamelessSheet", ["spritePos"] = 0, ["zOrder"] = 0}, id))");
    luaState.script(R"(
        local blackboard = {}
        blackboard.data = {}
        blackboard.primaryShot = {}
        blackboard.pID = id
        local function movement (velocity, speed, focusSpeed)
            local moveSpeed = input:keyDown(input:scancode("P1Focus")) and focusSpeed or speed
            local right = input:keyDown(input:scancode("P1Move_Right")) and 1 or 0
            local left = input:keyDown(input:scancode("P1Move_Left")) and 1 or 0
            local up = input:keyDown(input:scancode("P1Move_Up")) and 1 or 0
            local down = input:keyDown(input:scancode("P1Move_Down")) and 1 or 0
            velocity.velX, velocity.velY = (right - left) * moveSpeed, (down - up) * moveSpeed
        end
        local function onRevive()
            blackboard.createPrimaryShots()
        end
        local function onDeath ()
            local i = 1
            while blackboard.primaryShot[i] do
                manager:removeComponent("sprite", blackboard.primaryShot[i].id)
                manager:removeComponent("spawner", blackboard.primaryShot[i].id)
                i = i + 1
            end

        end
        local function primaryAtk (playerEntity, id, playerCmp)
            local timeReady = playerCmp.currentTime >= playerCmp.cooldown
            local keyPressed = input:keyDown(input:scancode("P1ShotPri"))
            local spawnCmps = {}
            if timeReady and keyPressed then
                local i = 1
                while spawnCmps and blackboard.primaryShot[i] do
                    local entity = manager:getEntity(blackboard.primaryShot[i].id)
                    local spawner = {}
                    entity:getComponent("spawner", spawner, manager)
                    if spawner.data then spawnCmps = nil break
                    else
                        local sp = {}
                        local bursts = {{}}
                        sp.bursts = bursts
                        sp.totalRuns = 1
                        sp.initialDelay = 0
                        bursts[1].runCount = 3
                        bursts[1].spawnsPerRun = 2
                        bursts[1].repeatRate = 200
                        bursts[1].initialDelay = 0
                        bursts[1].relativity = "Source"
                        bursts[1].posDir = "PosRad"
                        bursts[1].pos = {}
                        bursts[1].pos.dir = 85
                        bursts[1].pos.speed = 0.1
                        bursts[1].pos.deltaDir = 10
                        bursts[1].pos.dSpeed = 0
                        bursts[1].pos.persistDeltaDir = 0
                        bursts[1].velDir = "VelSpeed"
                        bursts[1].vel = {}
                        bursts[1].vel.speed = 200
                        bursts[1].vel.dSpeed = 0
                        bursts[1].vel.persistDSpeed = -20
                        bursts[1].spawnVelType = "AwayFromOrigin"
                        bursts[1].rotate = true
                        bursts[1].extraComponents = {{}}
                        local colli = {
                            ["componentName"] = "collider",
                            ["component"] = {
                                ["handlers"] = { ['Enemy'] = function (id1, id2) manager:destroyEntity(id1) manager:destroyEntity(id2) end},
                                ["group"] = "PlayerBullet",
                                ["collider"] = {
                                    ["type"] = "Circle",
                                    ["radius"] = 5
                                }
                            }
                        }
                        bursts[1].extraComponents[1][1] = {
                            ["componentName"] = "sprite",
                            ["component"] = {
                                ["fileName"] = "NamelessSheet",
                                ["spritePos"] = 5,
                                ["zOrder"] = 80

                            }

                        }
                        bursts[1].extraComponents[1][2] = {
                            ["componentName"] = "bounds",
                            ["component"] = {
                                ["xBehavior"] = "destroy",
                                ["yBehavior"] = "destroy",
                                ["minX"] = 0, ["minY"] = 0,
                                ["maxX"] = 360, ["maxY"] = 480

                            }

                        }
                        bursts[1].extraComponents[1][3] = colli
                        local pSid = blackboard.primaryShot[i].id
                        spawnCmps[i] = function() manager:addComponent("spawner", sp, pSid) end
                        i = i + 1
                    end
                end
                if spawnCmps then
                    local i = 1
                    while spawnCmps[i] do
                        spawnCmps[i]()
                        i = i + 1
                    end
                    playerCmp.currentTime = 0
                end
            end

        end

        blackboard.createPrimaryShots =
            function ()
                local pID = blackboard.pID
                blackboard.primaryShot.size = 4
                for i = 1, blackboard.primaryShot.size, 1 do
                    if not blackboard.primaryShot[i] then
                        blackboard.primaryShot[i] = {}
                    end
                    blackboard.primaryShot[i].id = manager:createEntity()
                    local xPos = 0
                    local yPos = 0
                    xPos = i == 1 and -20 or i == 2 and 20 or 0
                    yPos = i == 3 and -20 or i == 4 and 20 or 0
                    local pShotTF = {
                        ["parent"] = pID,
                        ["transform"] = {
                            ["translate"] = {x = xPos, y = yPos}
                        }
                    }
                    local pShotSpr = {
                        ["fileName"] = "NamelessSheet", ["zOrder"] = 0, ["spritePos"] = 2
                    }
                    manager:addComponent("bounds",
                    {   ["xBehavior"] = "wrap",
                        ["yBehavior"] = "wrap",
                        ["minX"] = 0, ["minY"] = 0,
                        ["maxX"] = 360, ["maxY"] = 480
                    }, blackboard.primaryShot[i].id)

                    manager:addComponent("transform", pShotTF, blackboard.primaryShot[i].id)
                    manager:addComponent("sprite", pShotSpr, blackboard.primaryShot[i].id)

                end
            end
        blackboard.createPrimaryShots()

        blackboard.update =
        function (dt, id)
            local playerEntity = manager:getEntity(id)
            local cmpTbl = {["velocity"] = {}, ["player"] = {}}
            playerEntity:getComponent("velocity", cmpTbl.velocity, manager)
            playerEntity:getComponent("player", cmpTbl.player, manager)
            local velocity = cmpTbl.velocity.data
            local playerCmp = cmpTbl.player.data
            movement(velocity, playerCmp.speed, playerCmp.focusSpeed)
            primaryAtk(playerEntity, id, playerCmp)
        end
        blackboard.movement = movement
        blackboard.onRevive = onRevive
        blackboard.onDeath = onDeath

        manager:addComponent("player",
                                {["speed"] = 80,
                                 ["focusSpeed"] = 45,
                                 ["cooldown"] = 680,
                                 ["blackboard"] = blackboard
                                }, id)
                            )");
    luaState.script(R"( manager:addComponent("transform",
                                                {["transform"] =
                                                    {["angle"] = 90, ["translate"] = {["x"] = 150, ["y"] = 300} }
                                                }, id)
                                            )");
    luaState.script(R"( manager:addComponent("bounds",
                                                {["xBehavior"] = "wrap",
                                                 ["yBehavior"] = "wrap",
                                                 ["minX"] = 0, ["minY"] = 0,
                                                 ["maxX"] = 300, ["maxY"] = 400}, id)
                                            )");
    luaState.script(R"(
        manager:addComponent("collider",
        {
            ["group"] = "Player",
            ["handlers"] = { ['Enemy'] = playerHit, ['EnemyBullet'] = playerHit },
            ["collider"] = {
                ["type"] = "Circle",
                ["radius"] = 3
            }
        }, id)
    )");

    /**/
    luaState.script(R"(
        --[[
        local pID = id
        id = manager:createEntity()
        manager:addComponent("transform",
        {
            ["parent"] = pID,
            ["transform"] = {
                ["angle"] = 0, ["translate"] = {x = 30, y = 0}
            }
        }, id)
        manager:addComponent("sprite",
        {
            ["fileName"] = "NamelessSheet", ["zOrder"] = 0, ["spritePos"] = 2

        }, id)
        manager:addComponent("bounds",
        {
            ["minX"] = 0, ["minY"] = 0,
            ["maxX"] = 300, ["maxY"] = 400}, id)
        id = manager:createEntity()
        manager:addComponent("transform",
        {
            ["parent"] = pID,
            ["transform"] = {
                ["angle"] = 0, ["translate"] = {x = 0, y = 30}
            }
        }, id)
        manager:addComponent("sprite",
        {
            ["fileName"] = "NamelessSheet", ["zOrder"] = 0, ["spritePos"] = 2

        }, id)
        manager:addComponent("bounds",
        {
            ["minX"] = 0, ["minY"] = 0,
            ["maxX"] = 300, ["maxY"] = 400}, id)
        id = manager:createEntity()
        manager:addComponent("transform",
        {
            ["parent"] = pID,
            ["transform"] = {
                ["angle"] = 0, ["translate"] = {x = 30, y = 0}
            }
        }, id)
        manager:addComponent("sprite",
        {
            ["fileName"] = "NamelessSheet", ["zOrder"] = 0, ["spritePos"] = 2

        }, id)
        manager:addComponent("bounds",
        {
            ["minX"] = 0, ["minY"] = 0,
            ["maxX"] = 300, ["maxY"] = 400}, id)
        id = manager:createEntity()
        manager:addComponent("transform",
        {
            ["parent"] = pID,
            ["transform"] = {
                ["angle"] = 0, ["translate"] = {x = 15, y = 15}
            }
        }, id)
        manager:addComponent("sprite",
        {
            ["fileName"] = "NamelessSheet", ["zOrder"] = 0, ["spritePos"] = 2

        }, id)
        manager:addComponent("bounds",
        {
            ["minX"] = 0, ["minY"] = 0,
            ["maxX"] = 300, ["maxY"] = 400}, id)
        --]]
    )");
    luaState.script(R"(
        ---[[
        for j = 1, 3, 1 do

            local tf = {}
            local sp = {}
            local bursts = {}
            id = manager:createEntity()
            tf.transform = {}
            tf.transform.translate = {["x"] = 300 - 60 * j, ["y"] = 40.0}
            tf.transform.angle = -90
            sp.totalRuns = -1
            sp.initialDelay = 2500
            local colli = {
                ["componentName"] = "collider",
                ["component"] = {
                    ["group"] = "EnemyBullet",
                    ["collider"] = {
                        ["type"] = "Circle",
                        ["radius"] = 5
                    }
                }
            }
            for i = 1, 4, 1 do
                bursts[i] = {}
                bursts[i].runCount = 1
                --bursts[i].spawnsPerRun = 1 + (i - 1) // 3 * 2
                bursts[i].repeatRate = 10 + (i-1) / 1.5 * 10
                bursts[i].initialDelay = 60 * (i % 2) + 60
                bursts[i].relativity = "Source"
                bursts[i].posDir = "PosRad"
                bursts[i].pos = {}
                bursts[i].pos.dir = 90 * i + -60
                bursts[i].pos.speed = 0.001
                bursts[i].pos.deltaDir = 6
                bursts[i].pos.dSpeed = 0
                bursts[i].pos.persistDeltaDir = -6
                if i % 2 == 0 then
                    bursts[i].pos.persistDeltaDir = -bursts[i].pos.persistDeltaDir
                    --bursts[i].pos.deltaDir = -bursts[i].pos.deltaDir
                end
                if i <= 4//2 then
                    bursts[i].spawnsPerRun = 3
                    bursts[i].pos.deltaDir = bursts[i].pos.deltaDir * (4//2 + 1.5 - i) * 1.250
                else
                    bursts[i].spawnsPerRun = 2
                    bursts[i].pos.persistDeltaDir = bursts[i].pos.persistDeltaDir * (i - 4//2) * 1.30
                end
                bursts[i].pos.persistDSpeed = 0
                bursts[i].velDir = "VelSpeed"
                bursts[i].vel = {}
                bursts[i].vel.speed = 60 - i * 4
                bursts[i].vel.dSpeed = 1.25 * i
                bursts[i].vel.persistDSpeed = 0
                bursts[i].spawnVelType = "AwayFromOrigin"
                if (i % 2 == 0) then bursts[i].initialDelay = 0 bursts[i].rotate = true end
                bursts[i].rotate = true
                if (i > 2) then bursts[i].extraComponents = bursts[i-2].extraComponents
                else bursts[i].extraComponents = {{}}
                end
            end
            bursts[1].initialDelay = 100
            bursts[3].initialDelay = 100
            bursts[1].extraComponents[1][1] = {
                ["componentName"] = "sprite",
                ["component"] = {
                    ["fileName"] = "NamelessSheet",
                    ["spritePos"] = 3,
                    ["zOrder"] = 80

                }

            }
            bursts[1].extraComponents[1][2] = {
                ["componentName"] = "bounds",
                ["component"] = {
                    ["xBehavior"] = "destroy",
                    ["yBehavior"] = "destroy",
                    ["minX"] = 0, ["minY"] = 0,
                    ["maxX"] = 300, ["maxY"] = 400

                }

            }
            bursts[1].extraComponents[1][3] = colli
            bursts[2].extraComponents[1][1] = {
                ["componentName"] = "sprite",
                ["component"] = {
                    ["fileName"] = "NamelessSheet",
                    ["spritePos"] = 5,
                    ["zOrder"] = 80

                }

            }
            bursts[2].extraComponents[1][2] = bursts[1].extraComponents[1][2]
            bursts[2].extraComponents[1][3] = bursts[1].extraComponents[1][3]

            sp.bursts = bursts
            local x = 0
            local y = 0
            if j == 1 then y = 0 x = -18 end
            if j == 2 then x = 12 y = 12 end
            if j == 3 then y = 0 x = 18 end
            manager:addComponent("velocity",
            {
                ["x"] = x * 2, ["y"] = y * 2
            }
            , id)
            manager:addComponent("transform", tf, id)
            manager:addComponent("spawner", sp, id)

            manager:addComponent("bounds",
            {
                ["minX"] = 60, ["minY"] = 0,
                ["maxX"] = 240, ["maxY"] = 80,
                ["xBehavior"] = "bounce", ["yBehavior"] = "bounce"
            }
            , id)
            manager:addComponent("sprite", { ["fileName"] = "NamelessSheet", ["spritePos"] = 0, ["zOrder"] = 0}, id)

            manager:addComponent("collider",
            {
                ["group"] = "Enemy",
                ["collider"] = {
                    ["type"] = "Circle",
                    ["radius"] = 12
                }

            }
            , id)
        end
        --]]
    )");
    /**/

}

void PlayState::handleInput() {

}

void PlayState::update(float rate) {
    auto beg = SDL_GetPerformanceCounter();
    manager.update(rate);
    auto ed = SDL_GetPerformanceCounter();
    //std::cout << "UPDATE - " << ((ed - beg) * 1000.f / SDL_GetPerformanceFrequency()) << std::endl;

}

void PlayState::render(float remainder) {
    renderSys.render(remainder);
}

PlayState::~PlayState() {
    window = nullptr;
    luaState["manager"] = sol::nil;
    luaState["input"] = sol::nil;
}
