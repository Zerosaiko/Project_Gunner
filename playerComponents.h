#ifndef PLAYERCOMPONENTS_H_INCLUDED
#define PLAYERCOMPONENTS_H_INCLUDED

#include <string>

struct PlayerCmp {

    static const std::string name;

    float speed;
    float focusSpeed;

    float aggro;

    uint8_t playerNumber;

};

#endif // PLAYERCOMPONENTS_H_INCLUDED
