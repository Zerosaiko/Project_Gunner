#ifndef RENDERABLE_H_INCLUDED
#define RENDERABLE_H_INCLUDED

#include <string>
#include "spriteSheet.h"
#include "component.h"

struct Renderable {
    std::string spriteName;
    int spritePos;
    int zOrder;
    SpriteSheet* sheet;
    static const std::string name;
    static const std::string getName();

    Renderable();

};

bool operator==(const Renderable& a, const Renderable& b);

bool operator!=(const Renderable& a, const Renderable& b);

bool operator<(const Renderable& a, const Renderable& b);

bool operator>(const Renderable& a, const Renderable& b);

bool operator<=(const Renderable& a, const Renderable& b);

bool operator>=(const Renderable& a, const Renderable& b);

#endif // RENDERABLE_H_INCLUDED
