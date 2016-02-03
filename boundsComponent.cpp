#include "boundsComponent.h"
#include "component.h"



Bounds::Bounds() : behavior(Bounds::Behavior::none), minX(0.f), minY(0.f), maxX(0.f), maxY(0.f) {
}

const std::string Bounds::name{"bounds"};

template<>
Bounds buildFromString<Bounds>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    Bounds b;
    if (str[pos] == "block")
        b.behavior = Bounds::Behavior::block;
    else if (str[pos] == "destroy")
        b.behavior = Bounds::Behavior::destroy;
    else if (str[pos] == "wrap")
        b.behavior = Bounds::Behavior::wrap;
    else if (str[pos] == "wrap_x")
        b.behavior = Bounds::Behavior::wrap_x;
    else if (str[pos] == "wrap_y")
        b.behavior = Bounds::Behavior::wrap_y;
    b.minX = buildFromString<float>(str, ++pos);
    b.minY = buildFromString<float>(str, ++pos);
    b.maxX = buildFromString<float>(str, ++pos);
    b.maxY = buildFromString<float>(str, ++pos);
    return b;
}
