#include "Orientation.h"
#include "component.h"

Orientation::Orientation() : angle(0.0f), scaleX(1.0f), scaleY(1.0f), flipX(1.0f), flipY(1.0f), hasOrigin(false) {

}

std::string Orientation::name{"orientation"};

template<>
Orientation buildFromString<Orientation>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    Orientation o;
    o.origin.x = o.origin.y = 0;
    while (pos < str.size()) {
        if (str[pos] == "angle") {
            o.angle = buildFromString<float>(str, ++pos);
        } else if (str[pos] == "flipX") {
            o.flipX = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "flipY") {
            o.flipY = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "origin") {
            o.origin.x = buildFromString<int32_t>(str, ++pos);
            o.origin.y = buildFromString<int32_t>(str, ++pos);
            o.hasOrigin = true;
        } else if (str[pos] == "scale") {
            o.scaleX = buildFromString<float>(str, ++pos);
            o.scaleY = buildFromString<float>(str, ++pos);
        }
        ++pos;
    }
    return o;
}
