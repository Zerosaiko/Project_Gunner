#include "renderable.h"

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
