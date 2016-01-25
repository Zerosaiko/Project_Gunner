#include "renderable.h"
#include <cstdlib>
#include <vector>

Renderable::Renderable() : spriteName("defaultsprite"), spritePos(0), zOrder(0), sheet(nullptr) {}

const std::string Renderable::name{"sprite"};

const std::string Renderable::getName() {
    return std::string{"sprite"};
}

bool operator==(const Renderable& a, const Renderable& b) {
    return a.sheet == b.sheet && a.zOrder == b.zOrder;
}

bool operator!=(const Renderable& a, const Renderable& b) {
    return !operator==(a, b);
}

bool operator<(const Renderable& a, const Renderable& b) {
    if (a.sheet != b.sheet)
        return a.sheet < b.sheet;
    return a.zOrder < b.zOrder;
}

bool operator>(const Renderable& a, const Renderable& b) {
    return operator<(b, a);
}

bool operator<=(const Renderable& a, const Renderable& b) {
    return !operator>(a, b);
}

bool operator>=(const Renderable& a, const Renderable& b) {
    return !operator<(a, b);
}

template<>
Renderable buildFromString<Renderable>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    Renderable r;
    r.spriteName = str[pos++];
    r.spritePos = std::atoi(str[pos++].c_str());
    r.zOrder = std::atoi(str[pos++].c_str());
    return r;
}
