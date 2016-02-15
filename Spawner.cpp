#include "Spawner.h"
#include <cmath>

static const float PI = acosf(-1.0f);

const std::string Spawner::name{"spawner"};

const std::string Spawner::getName() {
    return Spawner::name;
}

Spawner::Spawner() {}

template<>
Spawner buildFromString<Spawner>(std::vector<std::string> str, size_t pos) {
    Spawner s;
    std::cout << str[pos] << '\n';
    s.runCount = buildFromString<int32_t>(str, pos);
    s.spawnsPerRun = buildFromString<uint32_t>(str, ++pos);
    std::cout << str[pos] << '\n';
    s.repeatRate = buildFromString<float>(str, ++pos);
    std::cout << str[pos] << '\n';
    s.currentTime = s.repeatRate;
    s.relative = Spawner::Relative::Absolute;

    if (str[++pos] == "Source") {
        std::cout << str[pos] << '\n';
        s.relative = Spawner::Relative::Source;
        ++pos;
    } else if (str[pos] == "Player") {
        s.relative = Spawner::Relative::Player;
        std::cout << str[pos] << '\n';
        ++pos;
    }

    s.posDirection = Spawner::PointStyle::XY;

    s.velDirection = Spawner::PointStyle::XY;

    s.accelDirection = Spawner::PointStyle::XY;

    if (str[pos] == "PosXY") {
        std::cout << str[pos] << '\n';
        s.posDirection = Spawner::PointStyle::XY;
        s.position.xyVec.x = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        s.position.xyVec.y = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        s.position.xyVec.dx = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        s.position.xyVec.dy = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        ++pos;
    } else if (str[pos] == "PosRad") {
        s.posDirection = Spawner::PointStyle::Rad;
        std::cout << str[pos] << '\n';
        s.position.dirSpd.direction = buildFromString<float>(str, ++pos) * (PI / 180.0f); std::cout << str[pos] << '\n';
        s.position.dirSpd.deltaDirection = buildFromString<float>(str, ++pos) * (PI / 180.0f); std::cout << str[pos] << '\n';
        s.position.dirSpd.speed = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        s.position.dirSpd.dSpeed = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        ++pos;
    }

    if (str[pos] == "VelXY") {
        s.velDirection = Spawner::PointStyle::XY;
        std::cout << str[pos] << '\n';
        s.velocity.xyVec.x = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        s.velocity.xyVec.y = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        s.velocity.xyVec.dx = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        s.velocity.xyVec.dy = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        ++pos;
    } else if (str[pos] == "VelRad") {
        s.velDirection = Spawner::PointStyle::Rad;
        std::cout << str[pos] << '\n';
        s.velocity.dirSpd.direction = buildFromString<float>(str, ++pos) * (PI / 180.0f); std::cout << str[pos] << '\n';
        s.velocity.dirSpd.deltaDirection = buildFromString<float>(str, ++pos) * (PI / 180.0f); std::cout << str[pos] << '\n';
        s.velocity.dirSpd.speed = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        s.velocity.dirSpd.dSpeed = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        ++pos;
    } else if (str[pos] == "VelSpeed") {
        s.velDirection = Spawner::PointStyle::Speed;
        std::cout << str[pos] << '\n';
        s.velocity.speed = buildFromString<float>(str, ++pos); std::cout << str[pos] << '\n';
        ++pos;
    }

    if (str[pos] == "AccelXY") {
        s.accelDirection = Spawner::PointStyle::XY;
        std::cout << str[pos] << '\n';
        s.acceleration.xyVec.x = buildFromString<float>(str, ++pos);std::cout << str[pos] << '\n';
        s.acceleration.xyVec.y = buildFromString<float>(str, ++pos);std::cout << str[pos] << '\n';
        s.acceleration.xyVec.dx = buildFromString<float>(str, ++pos);std::cout << str[pos] << '\n';
        s.acceleration.xyVec.dy = buildFromString<float>(str, ++pos);std::cout << str[pos] << '\n';
        ++pos;
    } else if (str[pos] == "AccelRad") {
        s.accelDirection = Spawner::PointStyle::Rad;
        std::cout << str[pos] << '\n';
        s.acceleration.dirSpd.direction = buildFromString<float>(str, ++pos) * (PI / 180.0f); std::cout << str[pos] << '\n';
        s.acceleration.dirSpd.deltaDirection = buildFromString<float>(str, ++pos) * (PI / 180.0f);std::cout << str[pos] << '\n';
        s.acceleration.dirSpd.speed = buildFromString<float>(str, ++pos);std::cout << str[pos] << '\n';
        s.acceleration.dirSpd.dSpeed = buildFromString<float>(str, ++pos);std::cout << str[pos] << '\n';
        ++pos;
    } else if (str[pos] == "AccelSpeed") {
        s.accelDirection = Spawner::PointStyle::Speed;
        std::cout << str[pos] << '\n';
        s.acceleration.speed = buildFromString<float>(str, ++pos);std::cout << str[pos] << '\n';
        ++pos;
    }

    s.spawnPosition = Spawner::SpawnPos::Default;
    if (str[pos] == "AtPoints") {
        std::cout << str[pos] << '\n';
        uint32_t pointCount = buildFromString<uint32_t>(str,++pos);std::cout << str[pos] << '\n';
        s.spawnPoints.reserve(pointCount * 2);
        for(uint32_t i = 0; i < pointCount; ++i) {
            s.spawnPoints.push_back(buildFromString<float>(str, ++pos)); std::cout << str[pos] << '\n';
            s.spawnPoints.push_back(buildFromString<float>(str, ++pos)); std::cout << str[pos] << '\n';
        }
        ++pos;
    }

    s.spawnVelocity = Spawner::SpawnVel::None;
    if (str[pos] == "Default") {
        std::cout << str[pos] << '\n';
        s.spawnVelocity = Spawner::SpawnVel::Default;
        ++pos;
    } else if (str[pos] == "Aimed") {
        std::cout << str[pos] << '\n';
        s.spawnVelocity = Spawner::SpawnVel::Aimed;
        ++pos;
    } else if (str[pos] == "TowardOrigin") {
        std::cout << str[pos] << '\n';
        s.spawnVelocity = Spawner::SpawnVel::TowardOrigin;
        ++pos;
    } else if (str[pos] == "AwayFromOrigin") {
        std::cout << str[pos] << '\n';
        s.spawnVelocity = Spawner::SpawnVel::AwayFromOrigin;
        ++pos;
    } else if (str[pos] == "UseList") {
        std::cout << str[pos] << '\n';
        s.spawnVelocity = Spawner::SpawnVel::UseList;
        uint32_t pointCount = buildFromString<uint32_t>(str,++pos); std::cout << str[pos] << '\n';
        s.velocityList.reserve(pointCount * 2);
        for(uint32_t i = 0; i < pointCount; ++i) {
            s.velocityList.push_back(buildFromString<float>(str, ++pos)); std::cout << str[pos] << '\n';
            s.velocityList.push_back(buildFromString<float>(str, ++pos)); std::cout << str[pos] << '\n';
        }
        ++pos;
    }

    s.spawnAcceleration = Spawner::SpawnAccel::None;
    if (str[pos] == "Default") {
        std::cout << str[pos] << '\n';
        s.spawnAcceleration = Spawner::SpawnAccel::Default;
        ++pos;
    } else if (str[pos] == "Forward") {
        std::cout << str[pos] << '\n';
        s.spawnAcceleration = Spawner::SpawnAccel::Forward;
        ++pos;
    } else if (str[pos] == "Backward") {
        std::cout << str[pos] << '\n';
        s.spawnAcceleration = Spawner::SpawnAccel::Backward;
        ++pos;
    } else if (str[pos] == "Decel") {
        std::cout << str[pos] << '\n';
        s.spawnAcceleration = Spawner::SpawnAccel::Decel;
        ++pos;
    }

    std::cout << "cmpTypeCount" << str[pos] << '\n';
    s.cmpTypeCount = buildFromString<uint32_t>(str, pos++);
    s.addComponents.resize(s.cmpTypeCount);
    std::cout << str[pos] << '\n';
    std::cout << pos << '\t' << str.size() << '\n';
    while (pos < str.size() && str[pos] == "component") {
        std::cout << str[pos] << '\n';
        uint32_t entityComp = buildFromString<uint32_t>(str, ++pos); std::cout << str[pos] << '\n';
        s.addComponents.at(entityComp).push_back(str[++pos]); std::cout << str[pos] << '\n';
        ++pos;
    }

    return s;
}
