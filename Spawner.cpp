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

    s.runCount = buildFromString<int32_t>(str, pos);
    s.spawnsPerRun = buildFromString<uint32_t>(str, ++pos);

    s.repeatRate = buildFromString<float>(str, ++pos);

    s.initialDelay = buildFromString<float>(str, ++pos);

    s.currentTime = s.repeatRate - s.initialDelay;
    s.relative = Spawner::Relative::Absolute;

    if (str[++pos] == "Source") {

        s.relative = Spawner::Relative::Source;
        ++pos;
    } else if (str[pos] == "Player") {
        s.relative = Spawner::Relative::Player;

        ++pos;
    }

    s.posDirection = Spawner::PointStyle::XY;

    s.velDirection = Spawner::PointStyle::XY;

    s.accelDirection = Spawner::PointStyle::XY;

    if (str[pos] == "PosXY") {

        s.posDirection = Spawner::PointStyle::XY;
        s.position.xyVec.x = buildFromString<float>(str, ++pos);
        s.position.xyVec.y = buildFromString<float>(str, ++pos);
        s.position.xyVec.dx = buildFromString<float>(str, ++pos);
        s.position.xyVec.dy = buildFromString<float>(str, ++pos);
        s.position.xyVec.persistDx = buildFromString<float>(str, ++pos);
        s.position.xyVec.persistDy = buildFromString<float>(str, ++pos);
        ++pos;
    } else if (str[pos] == "PosRad") {
        s.posDirection = Spawner::PointStyle::Rad;

        s.position.dirSpd.direction = buildFromString<float>(str, ++pos) * (PI / 180.0f);
        s.position.dirSpd.speed = buildFromString<float>(str, ++pos);
        s.position.dirSpd.deltaDirection = buildFromString<float>(str, ++pos) * (PI / 180.0f);
        s.position.dirSpd.dSpeed = buildFromString<float>(str, ++pos);
        s.position.dirSpd.persistDeltaDirection = buildFromString<float>(str, ++pos) * (PI / 180.0f);
        s.position.dirSpd.persistDSpeed = buildFromString<float>(str, ++pos);
        ++pos;
    }

    if (str[pos] == "VelXY") {
        s.velDirection = Spawner::PointStyle::XY;

        s.velocity.xyVec.x = buildFromString<float>(str, ++pos);
        s.velocity.xyVec.y = buildFromString<float>(str, ++pos);
        s.velocity.xyVec.dx = buildFromString<float>(str, ++pos);
        s.velocity.xyVec.dy = buildFromString<float>(str, ++pos);
        s.velocity.xyVec.persistDx = buildFromString<float>(str, ++pos);
        s.velocity.xyVec.persistDy = buildFromString<float>(str, ++pos);
        ++pos;
    } else if (str[pos] == "VelRad") {
        s.velDirection = Spawner::PointStyle::Rad;

        s.velocity.dirSpd.direction = buildFromString<float>(str, ++pos) * (PI / 180.0f);
        s.velocity.dirSpd.speed = buildFromString<float>(str, ++pos);
        s.velocity.dirSpd.deltaDirection = buildFromString<float>(str, ++pos) * (PI / 180.0f);
        s.velocity.dirSpd.dSpeed = buildFromString<float>(str, ++pos);
        s.velocity.dirSpd.persistDeltaDirection = buildFromString<float>(str, ++pos) * (PI / 180.0f);
        s.velocity.dirSpd.persistDSpeed = buildFromString<float>(str, ++pos);
        ++pos;
    } else if (str[pos] == "VelSpeed") {
        s.velDirection = Spawner::PointStyle::Speed;
        s.velocity.speed.current = buildFromString<float>(str, ++pos);
        s.velocity.speed.delta = buildFromString<float>(str, ++pos);
        s.velocity.speed.persistDelta = buildFromString<float>(str, ++pos);
        ++pos;
    }

    if (str[pos] == "AccelXY") {
        s.accelDirection = Spawner::PointStyle::XY;

        s.acceleration.xyVec.x = buildFromString<float>(str, ++pos);
        s.acceleration.xyVec.y = buildFromString<float>(str, ++pos);
        s.acceleration.xyVec.dx = buildFromString<float>(str, ++pos);
        s.acceleration.xyVec.dy = buildFromString<float>(str, ++pos);
        ++pos;
    } else if (str[pos] == "AccelRad") {
        s.accelDirection = Spawner::PointStyle::Rad;

        s.acceleration.dirSpd.direction = buildFromString<float>(str, ++pos) * (PI / 180.0f);
        s.acceleration.dirSpd.speed = buildFromString<float>(str, ++pos);
        s.acceleration.dirSpd.deltaDirection = buildFromString<float>(str, ++pos) * (PI / 180.0f);
        s.acceleration.dirSpd.dSpeed = buildFromString<float>(str, ++pos);
        ++pos;
    } else if (str[pos] == "AccelSpeed") {
        s.accelDirection = Spawner::PointStyle::Speed;
        s.acceleration.speed.current = buildFromString<float>(str, ++pos);
        s.acceleration.speed.delta = buildFromString<float>(str, ++pos);
        s.acceleration.speed.persistDelta = buildFromString<float>(str, ++pos);
        ++pos;
    }

    s.spawnPosition = Spawner::SpawnPos::Default;
    if (str[pos] == "AtPoints") {

        uint32_t pointCount = buildFromString<uint32_t>(str,++pos);
        s.spawnPoints.reserve(pointCount * 2);
        for(uint32_t i = 0; i < pointCount; ++i) {
            s.spawnPoints.push_back(buildFromString<float>(str, ++pos));
            s.spawnPoints.push_back(buildFromString<float>(str, ++pos));
        }
        ++pos;
    }

    s.spawnVelocity = Spawner::SpawnVel::None;
    if (str[pos] == "Default") {

        s.spawnVelocity = Spawner::SpawnVel::Default;
        ++pos;
    } else if (str[pos] == "Aimed") {

        s.spawnVelocity = Spawner::SpawnVel::Aimed;
        ++pos;
    }  else if (str[pos] == "AwayFromPlayer") {

        s.spawnVelocity = Spawner::SpawnVel::AwayFromPlayer;
        ++pos;
    } else if (str[pos] == "AimedBySource") {

        s.spawnVelocity = Spawner::SpawnVel::AimedBySource;
        ++pos;
    } else if (str[pos] == "AimedAwayBySource") {

        s.spawnVelocity = Spawner::SpawnVel::AimedBySource;
        ++pos;
    } else if (str[pos] == "TowardOrigin") {

        s.spawnVelocity = Spawner::SpawnVel::TowardOrigin;
        ++pos;
    } else if (str[pos] == "AwayFromOrigin") {

        s.spawnVelocity = Spawner::SpawnVel::AwayFromOrigin;
        ++pos;
    } else if (str[pos] == "UseList") {

        s.spawnVelocity = Spawner::SpawnVel::UseList;
        uint32_t pointCount = buildFromString<uint32_t>(str,++pos);
        s.velocityList.reserve(pointCount * 2);
        for(uint32_t i = 0; i < pointCount; ++i) {
            s.velocityList.push_back(buildFromString<float>(str, ++pos));
            s.velocityList.push_back(buildFromString<float>(str, ++pos));
        }
        ++pos;
    }

    s.spawnAcceleration = Spawner::SpawnAccel::None;
    if (str[pos] == "Default") {

        s.spawnAcceleration = Spawner::SpawnAccel::Default;
        ++pos;
    } else if (str[pos] == "Forward") {

        s.spawnAcceleration = Spawner::SpawnAccel::Forward;
        ++pos;
    } else if (str[pos] == "Backward") {

        s.spawnAcceleration = Spawner::SpawnAccel::Backward;
        ++pos;
    } else if (str[pos] == "Decel") {

        s.spawnAcceleration = Spawner::SpawnAccel::Decel;
        ++pos;
    }

    s.cmpTypeCount = buildFromString<uint32_t>(str, pos++);
    s.addComponents.resize(s.cmpTypeCount);

    while (pos < str.size() && str[pos] == "component") {

        uint32_t entityComp = buildFromString<uint32_t>(str, ++pos);
        s.addComponents.at(entityComp).emplace_back(str[++pos]);
        ++pos;
    }

    return s;
}
