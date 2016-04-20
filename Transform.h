#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "SDL.h"
#include <string>

struct Transform {

    Transform();

    static const std::string name;

    float angle, scaleX, scaleY;
    bool flipX, flipY;
    SDL_Point origin;
};

#endif // TRANSFORM_H
