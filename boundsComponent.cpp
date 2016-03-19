#include "boundsComponent.h"
#include "component.h"



Bounds::Bounds() : xBehavior(Bounds::Behavior::none), yBehavior(Bounds::Behavior::none), minX(0.f), minY(0.f), maxX(0.f), maxY(0.f) {
}

const std::string Bounds::name{"bounds"};

template<>
Bounds buildFromString<Bounds>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    Bounds b;
    if (str[pos] == "block")
        b.xBehavior = Bounds::Behavior::block;
    else if (str[pos] == "destroy")
        b.xBehavior = Bounds::Behavior::destroy;
    else if (str[pos] == "wrap")
        b.xBehavior = Bounds::Behavior::wrap;
    else if (str[pos] == "bounce")
        b.xBehavior = Bounds::Behavior::bounce;

    if (str[++pos] == "block")
        b.yBehavior = Bounds::Behavior::block;
    else if (str[pos] == "destroy")
        b.yBehavior = Bounds::Behavior::destroy;
    else if (str[pos] == "wrap")
        b.yBehavior = Bounds::Behavior::wrap;
    else if (str[pos] == "bounce")
        b.yBehavior = Bounds::Behavior::bounce;

    b.minX = buildFromString<float>(str, ++pos);
    b.minY = buildFromString<float>(str, ++pos);
    b.maxX = buildFromString<float>(str, ++pos);
    b.maxY = buildFromString<float>(str, ++pos);

    if (++pos < str.size()) {
        if (str[pos] == "none") {
            b.limitType = Bounds::LimitType::none;
        } else if (str[pos] == "timeLimit") {
            b.limitType = Bounds::LimitType::time;
            b.timeLimit = buildFromString<float>(str, ++pos);
        } else if (str[pos] == "boundLimit") {
            b.limitType = Bounds::LimitType::boundLimit;
            b.boundLimit = buildFromString<uint32_t>(str, ++pos);
        }
    }

    if (++pos < str.size()) {
        if (str[pos] == "none") {
            b.postLimit = Bounds::PostLimitBehavior::none;
        } else if (str[pos] == "destroy") {
            b.postLimit = Bounds::PostLimitBehavior::destroy;
        } else if (str[pos] == "change") {
            b.postLimit = Bounds::PostLimitBehavior::change;
            while (++pos < str.size()) {
                b.changeBound.emplace_back(str[pos]);
            }
        }
    }

    return b;
}
