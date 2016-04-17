#include "shieldComponent.h"
#include "component.h"

const std::string Shield::name{"shield"};


Shield::Shield() : timeLimit(0){

}

Shield::Shield(float timeLimit) : timeLimit(timeLimit){

}

template<>
Shield buildFromString<Shield>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    Shield s;
    if (pos < str.size())
        s.timeLimit = buildFromString<float>(str, pos);

    return s;
}
