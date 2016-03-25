#include "renderable.h"
#include <cstdlib>
#include <vector>

Sprite::Sprite() : spriteName("defaultsprite"), spritePos(0), zOrder(0), sheet(nullptr) {}

const std::string Sprite::name{"sprite"};

const std::string Sprite::getName() {
    return std::string{"sprite"};
}

bool operator==(const Sprite& a, const Sprite& b) {
    return a.sheet == b.sheet && a.zOrder == b.zOrder;
}

bool operator!=(const Sprite& a, const Sprite& b) {
    return !operator==(a, b);
}

bool operator<(const Sprite& a, const Sprite& b) {
    if (a.sheet != b.sheet)
        return a.sheet < b.sheet;
    return a.zOrder < b.zOrder;
}

bool operator>(const Sprite& a, const Sprite& b) {
    return operator<(b, a);
}

bool operator<=(const Sprite& a, const Sprite& b) {
    return !operator>(a, b);
}

bool operator>=(const Sprite& a, const Sprite& b) {
    return !operator<(a, b);
}

template<>
Sprite buildFromString<Sprite>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    Sprite r;
    r.spriteName = str[pos++];
    r.spritePos = std::atoi(str[pos++].c_str());
    r.zOrder = std::atoi(str[pos++].c_str());
    return r;
}
