#include "renderable.h"
#include <cstdlib>
#include <vector>

Sprite::Sprite() : spriteName("defaultsprite"), spritePos(0), zOrder(0) {}

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
    if (a.zOrder != b.zOrder)
        return a.zOrder < b.zOrder;
    return a.sheet < b.sheet;
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
Sprite buildFromLua<Sprite>(sol::object& obj) {
    sol::table tbl = obj;
    Sprite r;
    r.spriteName = tbl["fileName"];
    r.spritePos = tbl["spritePos"];
    r.zOrder = tbl["zOrder"];
    return r;
}
