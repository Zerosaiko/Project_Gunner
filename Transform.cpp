#include "Transform.h"
#include "component.h"

Transform::Transform() : angle(0.0f), scaleX(1.0f), scaleY(1.0f), flipX(false), flipY(false), origin{0, 0} {

}

const std::string Transform::name{"transform"};

template<>
Transform buildFromString<Transform>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    Transform o;
    while (pos < str.size()) {
        if (str[pos] == "angle") {
            o.angle = buildFromString<float>(str, ++pos);
        } else if (str[pos] == "flipX") {
            o.flipX = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "flipY") {
            o.flipY = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "origin") {
            o.origin.x = buildFromString<int32_t>(str, ++pos);
            o.origin.y = buildFromString<int32_t>(str, pos);
        } else if (str[pos] == "scale") {
            o.scaleX = buildFromString<float>(str, ++pos);
            o.scaleY = buildFromString<float>(str, pos);
        }
    }
    return o;
}
