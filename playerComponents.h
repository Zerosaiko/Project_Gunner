#ifndef PLAYERCOMPONENTS_H_INCLUDED
#define PLAYERCOMPONENTS_H_INCLUDED

#include <string>
#include "sol.hpp"

struct PlayerCmp {

    static const std::string name;

    float speed;
    float focusSpeed;

    uint8_t aggro;

    uint8_t playerNumber;

    float cooldown;
    float currentTime;

    bool alive;
    float deathTimer;

    uint8_t lives;

    sol::table blackboard;

    PlayerCmp();

};

#endif // PLAYERCOMPONENTS_H_INCLUDED
