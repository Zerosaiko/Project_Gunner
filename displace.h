#ifndef DISPLACE_H_INCLUDED
#define DISPLACE_H_INCLUDED

#include "component.h"

struct Position {

    static const std::string name;
    float posX;
    float posY;
    float pastPosX;
    float pastPosY;

};

struct Velocity {
    static const std::string name;
    float velX;
    float velY;
};

#endif // DISPLACE_H_INCLUDED
