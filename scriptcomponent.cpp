#include "scriptcomponent.h"
#include <iostream>

const std::string Script::name{"script"};

template<>
Script buildFromString<Script>(std::vector<std::string> str, std::vector<std::string>::size_type pos) {
    Script s;
    s.updateRate = buildFromString<float>(str, pos);
    for (auto token = str.begin() + pos; token != str.end(); ++token) {
        s.tokenizedScript.push_back(*token);
    }
    return s;
}
