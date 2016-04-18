#ifndef HEALTH_H_INCLUDED
#define HEALTH_H_INCLUDED

#include <string>
#include <cstdint>

struct Health {

    static const std::string name;

    int32_t max;
    int32_t current;

    Health();

    Health(int32_t max);

    Health(int32_t max, int32_t current);

};

struct HealthRegen {

    static const std::string name;

    float rate;
    float cooldown;
    float currentTime;
    int32_t amount;

    HealthRegen();

    HealthRegen(float rate, int32_t amount);

    HealthRegen(float rate, float cooldown, int32_t amount);

};

#endif // HEALTH_H_INCLUDED
