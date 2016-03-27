#include "playerComponents.h"
#include "component.h"

const std::string PlayerCmp::name{"player"};

template<>
PlayerCmp buildFromString<PlayerCmp>(std::vector<std::string> str, std::string::size_type pos) {
    PlayerCmp p;
    p.speed = buildFromString<float>(str, pos);
    p.focusSpeed = buildFromString<float>(str, ++pos);
    p.aggro = 0.0f;
    p.playerNumber = 0;
    return p;
}
