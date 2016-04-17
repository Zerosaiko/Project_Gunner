#include "playerComponents.h"
#include "component.h"

PlayerCmp::PlayerCmp() : speed(0), focusSpeed(0), aggro(0), playerNumber(0), alive(true), deathTimer(3000.0f), lives(3) {}

const std::string PlayerCmp::name{"player"};

template<>
PlayerCmp buildFromString<PlayerCmp>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    PlayerCmp p;
    p.speed = buildFromString<float>(str, pos);
    p.focusSpeed = buildFromString<float>(str, pos);
    return p;
}
