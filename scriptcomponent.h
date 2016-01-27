#ifndef SCRIPTCOMPONENT_H_INCLUDED
#define SCRIPTCOMPONENT_H_INCLUDED

#include <string>
#include <vector>
#include "component.h"

//  contains a "script" that will be run by a ScriptSystem
struct Script {
    static const std::string name;
    float updateRate;
    float dt;
    std::vector<std::string> tokenizedScript;
};

#endif // SCRIPTCOMPONENT_H_INCLUDED
