#include "displace.h"
#include <iostream>
#include <locale>

const std::string Position::name{"position"};

template<>
Position buildFromString<Position>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    Position p;
    if (pos < str.size() && str.size() >= 3) {
        p.pastPosX = p.posX = buildFromString<float>(str, pos++);
        p.pastPosY = p.posY = buildFromString<float>(str, pos);
    }
    return p;
}

const std::string Velocity::name{"velocity"};

template<>
Velocity buildFromString<Velocity>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    Velocity v;
    if (pos < str.size() && str.size() >= 3) {
        v.velX = buildFromString<float>(str, pos++);
        v.velY = buildFromString<float>(str, pos);
    }
    return v;
}
