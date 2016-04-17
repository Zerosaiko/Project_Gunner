#ifndef ANIMATION_H_INCLUDED
#define ANIMATION_H_INCLUDED

#include <string>
#include <vector>
#include <cstdint>

struct Animation {

    static const std::string name;

    enum EndBehavior : uint8_t {
        None = 0,
        Loop
    };

    std::vector<uint32_t> frames;
    std::vector<float> frameLengths;
    uint32_t currentIdx;
    float currentTime;
    uint32_t loopIdx;
    EndBehavior endBehavior;

    Animation();

};

#endif // ANIMATION_H_INCLUDED
