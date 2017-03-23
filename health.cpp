#include "health.h"
#include "component.h"

const std::string Health::name{"health"};

Health::Health() :max(0), current(0) {}

Health::Health(int32_t max) : max(max), current(max) {}

Health::Health(int32_t max, int32_t current) :max(max), current(current) {}

template<>
Health buildFromLua<Health>(sol::object& obj) {
    sol::table tbl = obj;
    int32_t max = tbl["max"];

    return Health{max, tbl["current"].get_or(max)};
}

const std::string HealthRegen::name{"healthRegen"};

HealthRegen::HealthRegen() : HealthRegen(0.0, 0) {};

HealthRegen::HealthRegen(float rate, int32_t amount, float cooldown, uint32_t tickLimit) : currentTime(0.0f), rate(rate), amount(0), cooldown(cooldown) {};

template<>
HealthRegen buildFromLua<HealthRegen>(sol::object& obj) {
    sol::table tbl = obj;
    return HealthRegen{tbl["tickRate"].get_or(0.0f), tbl["amount"].get_or(0), tbl["cooldown"].get_or(0.0f), tbl["tickLimit"].get_or(0u)};
}

