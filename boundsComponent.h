#ifndef BOUNDSCOMPONENT_H_INCLUDED
#define BOUNDSCOMPONENT_H_INCLUDED

#include <string>

struct Bounds {

    Bounds();

    static const std::string name;

    enum class Behavior {
        none,
        block,
        wrap,
        wrap_x,
        wrap_y,
        destroy

    };

    Behavior behavior;
    float minX;
    float minY;
    float maxX;
    float maxY;

};

#endif // BOUNDSCOMPONENT_H_INCLUDED
