#ifndef SCRIPTCOMPONENT_H_INCLUDED
#define SCRIPTCOMPONENT_H_INCLUDED

#include <string>
#include <vector>
#include "component.h"

struct Script {
    static const std::string name;
    std::vector<std::string> tokenizedScript;
    int i = 0;
};

#endif // SCRIPTCOMPONENT_H_INCLUDED
