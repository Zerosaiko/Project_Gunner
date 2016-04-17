#ifndef PLAYERCOMPONENTS_H_INCLUDED
#define PLAYERCOMPONENTS_H_INCLUDED

#include <string>

struct PlayerCmp {

    static const std::string name;

    float speed;
    float focusSpeed;

    uint8_t aggro;

    uint8_t playerNumber;

    bool alive;
    float deathTimer;

    uint8_t lives;

    PlayerCmp();

};

#endif // PLAYERCOMPONENTS_H_INCLUDED
