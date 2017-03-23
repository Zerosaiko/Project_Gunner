#include "boundsComponent.h"
#include "component.h"



Bounds::Bounds() : xBehavior(Bounds::Behavior::none), yBehavior(Bounds::Behavior::none), minX(0.f), minY(0.f), maxX(0.f), maxY(0.f),
    limitType(Bounds::LimitType::none), postLimit(Bounds::PostLimitBehavior::none) {}

const std::string Bounds::name{"bounds"};

template<>
Bounds buildFromLua<Bounds>(sol::object& obj) {
    Bounds b;
    sol::table tbl{obj};
    std::string xBehavior = tbl["xBehavior"];
    std::string yBehavior = tbl["yBehavior"];
    if (xBehavior == "block")
        b.xBehavior = Bounds::Behavior::block;
    else if (xBehavior == "destroy")
        b.xBehavior = Bounds::Behavior::destroy;
    else if (xBehavior == "wrap")
        b.xBehavior = Bounds::Behavior::wrap;
    else if (xBehavior == "bounce")
        b.xBehavior = Bounds::Behavior::bounce;

    if (yBehavior == "block")
        b.yBehavior = Bounds::Behavior::block;
    else if (yBehavior == "destroy")
        b.yBehavior = Bounds::Behavior::destroy;
    else if (yBehavior == "wrap")
        b.yBehavior = Bounds::Behavior::wrap;
    else if (yBehavior == "bounce")
        b.yBehavior = Bounds::Behavior::bounce;

    b.minX = tbl["minX"];
    b.minY = tbl["minY"];
    b.maxX = tbl["maxX"];
    b.maxY = tbl["maxY"];
    {
        using namespace std;
        if (b.minX > b.maxX) swap(b.minX, b.maxX);
        if (b.minY > b.maxY) swap(b.minY, b.maxY);
    }
    std::string limitType{tbl["limitType"].get_or<std::string>("")};
    if (limitType == "timeLimit") {
        b.limitType = Bounds::LimitType::time;
        b.timeLimit = (float)tbl["timeLimit"];
    }
    else if (limitType == "boundsLimit") {
        b.limitType = Bounds::LimitType::boundsLimit;
        b.boundsLimit.x = tbl["boundsLimit"]["x"];
        b.boundsLimit.y = tbl["boundsLimit"]["y"];
    }
    std::string postLimitBehavior{tbl["postLimitBehavior"].get_or<std::string>("")};
    if (postLimitBehavior == "destroy") {
        b.postLimit = Bounds::PostLimitBehavior::destroy;
    }
    else if (postLimitBehavior == "change") {
        b.postLimit = Bounds::PostLimitBehavior::change;
        b.changeBounds  = tbl["changeBounds"];
    }

    return b;
}
