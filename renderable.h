#ifndef RENDERABLE_H_INCLUDED
#define RENDERABLE_H_INCLUDED

#include <string>
#include "spriteSheet.h"
#include "component.h"

struct Sprite {
    std::string spriteName;
    uint32_t spritePos;
    int16_t zOrder;
    SpriteSheet* sheet;
    static const std::string name;
    static const std::string getName();

    Sprite();

};

bool operator==(const Sprite& a, const Sprite& b);

bool operator!=(const Sprite& a, const Sprite& b);

bool operator<(const Sprite& a, const Sprite& b);

bool operator>(const Sprite& a, const Sprite& b);

bool operator<=(const Sprite& a, const Sprite& b);

bool operator>=(const Sprite& a, const Sprite& b);

#endif // RENDERABLE_H_INCLUDED
