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

    float currentTime;
    float rate;
    int32_t amount;
    float cooldown;
    uint32_t tickLimit;

    HealthRegen();

    HealthRegen(float rate, int32_t amount, float cooldown = 0.0f, uint32_t tickLimit = 0);
};

#endif // HEALTH_H_INCLUDED
