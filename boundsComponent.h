#ifndef BOUNDSCOMPONENT_H_INCLUDED
#define BOUNDSCOMPONENT_H_INCLUDED

#include <string>
#include <vector>

struct Bounds {

    Bounds();

    static const std::string name;

    enum class Behavior {
        none,
        block,
        wrap,
        bounce,
        destroy

    };

    enum class LimitType {
        none,
        time,
        boundLimit
    };

    enum class PostLimitBehavior {
        none,
        change,
        destroy
    };

    Behavior xBehavior;
    Behavior yBehavior;
    float minX;
    float minY;
    float maxX;
    float maxY;
    LimitType limitType;
    union {
        float timeLimit;
        uint32_t boundLimit;
    };
    PostLimitBehavior postLimit;
    std::vector<std::string> changeBound;

};

#endif // BOUNDSCOMPONENT_H_INCLUDED
