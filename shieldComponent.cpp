#include "shieldComponent.h"
#include "component.h"

const std::string Shield::name{"shield"};


Shield::Shield() : timeLimit(0){

}

Shield::Shield(float timeLimit) : timeLimit(timeLimit){

}


template<>
Shield buildFromLua<Shield>(sol::object& obj) {
    return Shield{obj.as<float>()};
}
