#include "animation.h"
#include "component.h"

const std::string Animation::name{"animation"};

Animation::Animation() : currentIdx(0), currentTime(0), loopIdx(0), endBehavior(Animation::EndBehavior::None) {

}

template<>
Animation buildFromString<Animation>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    Animation a;
    if (str[pos] == "Loop") {
        a.endBehavior = Animation::EndBehavior::Loop;
        a.loopIdx = buildFromString<uint32_t>(str, ++pos);
    }
    uint32_t frameCount = 0;
    frameCount = buildFromString<uint32_t>(str, pos);
    for (uint32_t i = 0; i < frameCount && pos < str.size(); ++i) {
        a.frames.emplace_back(buildFromString<uint32_t>(str, pos));
        a.frameLengths.emplace_back(buildFromString<float>(str, pos));
    }

    return a;
}
