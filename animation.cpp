#include "animation.h"
#include "component.h"

const std::string Animation::name{"animation"};

Animation::Animation() : currentIdx(0), currentTime(0), loopIdx(0), endBehavior(Animation::EndBehavior::None) {

}
template<>
Animation buildFromLua<Animation>(sol::object& obj) {
    Animation a;
    sol::table tbl = obj;
    if (tbl["endBehavior"] == "Loop") {
        a.endBehavior = Animation::EndBehavior::Loop;
        a.loopIdx = (uint32_t)tbl["endBehavior"]["loopIndex"];
    }
    sol::table frames = tbl["frames"];
    sol::table frameLengths = tbl["frameLengths"];
    for (auto it = frames.begin(); it != frames.end(); ++it) {
        a.frames.emplace_back((*it).second.as<uint32_t>());
    }
    for (auto it = frameLengths.begin(); it != frames.end(); ++it) {
        a.frameLengths.emplace_back((*it).second.as<float>());
    }

    return a;
}
