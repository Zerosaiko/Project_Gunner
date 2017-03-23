#include "playerComponents.h"
#include "component.h"

PlayerCmp::PlayerCmp() : speed(0), focusSpeed(0), aggro(0), playerNumber(0), cooldown(1.0f), currentTime(0), alive(true), deathTimer(3000.0f), lives(3) {}

const std::string PlayerCmp::name{"player"};

template<>
PlayerCmp buildFromLua<PlayerCmp>(sol::object& obj) {
    sol::table tbl = obj;
    PlayerCmp p;
    p.speed = tbl["speed"].get_or(0.0f);
    p.focusSpeed = tbl["focusSpeed"].get_or(0.0f);
    p.cooldown = tbl["cooldown"].get_or(1.0f);
    p.blackboard = tbl["blackboard"];
    return p;
}

