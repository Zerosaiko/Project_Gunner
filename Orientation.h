#ifndef ORIENTATION_H
#define ORIENTATION_H

#include "SDL.h"
#include <string>

struct Orientation {

    Orientation();

    static std::string name;

    float angle, scaleX, scaleY;
    bool flipX, flipY;
    bool hasOrigin;
    SDL_Point origin;
};

#endif // ORIENTATION_H
