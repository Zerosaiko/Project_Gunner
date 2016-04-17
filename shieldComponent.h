#ifndef SHIELDCOMPONENT_H_INCLUDED
#define SHIELDCOMPONENT_H_INCLUDED

#include <string>

struct Shield {

    static const std::string name;

    float timeLimit;

    Shield();

    Shield(float timeLimit);

};

#endif // SHIELDCOMPONENT_H_INCLUDED
