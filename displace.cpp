#include "displace.h"
#include <iostream>
#include <locale>

const std::string Position::name{"position"};

template<>
Position buildFromLua<Position>(sol::object& obj) {
    Position p;
    sol::table tbl = obj;
    p.pastPosX = p.posX = tbl["x"].get_or(0.0f);
    p.pastPosY = p.posY = tbl["y"].get_or(0.0f);
    return p;
}

const std::string Velocity::name{"velocity"};

template<>
Velocity buildFromLua<Velocity>(sol::object& obj) {
    Velocity v;
    sol::table tbl = obj;
    v.velX = tbl["x"].get_or(0.0f);
    v.velY = tbl["y"].get_or(0.0f);
    return v;
}
