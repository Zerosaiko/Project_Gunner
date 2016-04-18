#include "health.h"
#include "component.h"

const std::string Health::name{"health"};

Health::Health() :max(0), current(0) {}

Health::Health(int32_t max) : max(max), current(max) {}

Health::Health(int32_t max, int32_t current) :max(max), current(current) {}

template<>
Health buildFromString<Health>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    Health h{buildFromString<int32_t>(str, pos)};
    if (pos < str.size())
        h.current = buildFromString<int32_t>(str, ++pos);
    return h;
}

const std::string HealthRegen::name{"healthRegen"};

HealthRegen::HealthRegen() : rate(0.0f), currentTime(0.0f), amount(0) {};

HealthRegen::HealthRegen(float rate, int32_t amount) : rate(rate), currentTime(0.0f), amount(0) {};

HealthRegen::HealthRegen(float rate, float cooldown, int32_t amount) : rate(rate), cooldown(cooldown), currentTime(0.0f), amount(0) {};

template<>
HealthRegen buildFromString<HealthRegen>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    HealthRegen h;
    h.rate = buildFromString<float>(str, pos);
    h.cooldown = buildFromString<float>(str, pos);
    h.amount = buildFromString<int32_t>(str, pos);
    return h;
}
