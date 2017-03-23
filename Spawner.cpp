#include "Spawner.h"
#include <cmath>

static const float PI = acosf(-1.0f);

Spawner::Spawner() : totalRunCount(0), currentTime(0.0f), currentBurst(0.0f) {

}

const std::string Spawner::name{"spawner"};

const std::string Spawner::getName() {
    return Spawner::name;
}

Spawner::Burst::~Burst() {

    for(auto& cmpVec : addComponents) {
        for(auto& cmp : cmpVec) {
            cmp = sol::nil;
        }
    }
}

template<>
Spawner::Burst buildFromLua<Spawner::Burst>(sol::object& obj) {
    sol::table tbl = obj;
    Spawner::Burst s;

    //s.totalRunCount = tbl["totalRuns"];

    s.runs = 0;
    s.runCount = tbl["runCount"];
    s.spawnsPerRun = tbl["spawnsPerRun"];

    s.repeatRate = tbl["repeatRate"];

    s.initialDelay = tbl["initialDelay"];

    s.rotate = tbl["rotate"].get_or(false);

    s.relative = Spawner::Relative::Absolute;

    std::string relative = tbl["relativity"];
    if (relative == "Source") {

        s.relative = Spawner::Relative::Source;

    } else if (relative == "Player") {
        s.relative = Spawner::Relative::Player;

    }

    s.posDirection = Spawner::PointStyle::XY;

    s.velDirection = Spawner::PointStyle::XY;

    s.accelDirection = Spawner::PointStyle::XY;

    std::string posDirection = tbl["posDir"],
    velDirection = tbl["velDir"], accelDirection = tbl["accDir"];
    auto pos = tbl["pos"];
    auto vel = tbl["vel"];
    auto acc = tbl["acc"];
    if (posDirection == "PosXY") {

        s.posDirection = Spawner::PointStyle::XY;
        s.position.xyVec.x = pos["x"];
        s.position.xyVec.y = pos["y"];
        s.position.xyVec.dx = pos["dx"];
        s.position.xyVec.dy = pos["dy"];
        s.position.xyVec.persistDx = pos["persistDx"];
        s.position.xyVec.persistDy = pos["persistDy"];
    } else if (posDirection == "PosRad") {
        s.posDirection = Spawner::PointStyle::Rad;

        s.position.dirSpd.direction = (float)pos["dir"] * (PI / 180.0f);
        s.position.dirSpd.speed = pos["speed"];
        s.position.dirSpd.deltaDirection = (float)pos["deltaDir"] * (PI / 180.0f);
        s.position.dirSpd.dSpeed = pos["dSpeed"];
        s.position.dirSpd.persistDeltaDirection = (float)pos["persistDeltaDir"] * (PI / 180.0f);
        s.position.dirSpd.persistDSpeed = pos["persistDSpeed"];
    }

    if (velDirection == "VelXY") {
        s.velDirection = Spawner::PointStyle::XY;

        s.velocity.xyVec.x = vel["x"];
        s.velocity.xyVec.y = vel["y"];
        s.velocity.xyVec.dx = vel["dx"];
        s.velocity.xyVec.dy = vel["dy"];
        s.velocity.xyVec.persistDx = vel["persistDx"];
        s.velocity.xyVec.persistDy = vel["persistDy"];
    } else if (velDirection == "VelRad") {
        s.velDirection = Spawner::PointStyle::Rad;

        s.velocity.dirSpd.direction = (float)vel["dir"] * (PI / 180.0f);
        s.velocity.dirSpd.speed = vel["speed"];
        s.velocity.dirSpd.deltaDirection = (float)vel["deltaDir"] * (PI / 180.0f);
        s.velocity.dirSpd.dSpeed = vel["dSpeed"];
        s.velocity.dirSpd.persistDeltaDirection = (float)vel["persistDeltaDir"] * (PI / 180.0f);
        s.velocity.dirSpd.persistDSpeed = vel["persistDSpeed"];
    } else if (velDirection == "VelSpeed") {
        s.velDirection = Spawner::PointStyle::Speed;
        s.velocity.speed.current = vel["speed"];
        s.velocity.speed.delta = vel["dSpeed"];
        s.velocity.speed.persistDelta = vel["persistDSpeed"];
    }

    if (accelDirection == "AccelXY") {
        s.accelDirection = Spawner::PointStyle::XY;

        s.acceleration.xyVec.x = acc["x"];
        s.acceleration.xyVec.y = acc["y"];
        s.acceleration.xyVec.dx = acc["dx"];
        s.acceleration.xyVec.dy = acc["dy"];
    } else if (accelDirection == "AccelRad") {
        s.accelDirection = Spawner::PointStyle::Rad;

        s.acceleration.dirSpd.direction = (float)acc["dir"] * (PI / 180.0f);
        s.acceleration.dirSpd.speed = acc["speed"];
        s.acceleration.dirSpd.deltaDirection = (float)acc["deltaDir"] * (PI / 180.0f);
        s.acceleration.dirSpd.dSpeed = acc["dSpeed"];
    } else if (accelDirection == "AccelSpeed") {
        s.accelDirection = Spawner::PointStyle::Speed;
        s.acceleration.speed.current = acc["speed"];
        s.acceleration.speed.delta = acc["dSpeed"];
        s.acceleration.speed.persistDelta = acc["persistDSpeed"];
    }

    s.spawnPosition = Spawner::SpawnPos::Default;
    std::string spawnPosType = tbl["spawnPosType"];
    if (spawnPosType == "AtPoints") {

        sol::table pointsTable = tbl["spawnPos"]["points"];
        for (auto it = pointsTable.begin(); it != pointsTable.end(); ++it) {
            s.spawnPoints.push_back((*it).second.as<float>());
        }
    }

    s.spawnVelocity = Spawner::SpawnVel::Default;
    std::string spawnVelType = tbl["spawnVelType"];
    if (spawnVelType == "Aimed") {

        s.spawnVelocity = Spawner::SpawnVel::Aimed;
    }  else if (spawnVelType == "AwayFromPlayer") {

        s.spawnVelocity = Spawner::SpawnVel::AwayFromPlayer;
    } else if (spawnVelType == "AimedBySource") {

        s.spawnVelocity = Spawner::SpawnVel::AimedBySource;
    } else if (spawnVelType == "AimedAwayBySource") {

        s.spawnVelocity = Spawner::SpawnVel::AimedBySource;
    } else if (spawnVelType == "TowardOrigin") {

        s.spawnVelocity = Spawner::SpawnVel::TowardOrigin;
    } else if (spawnVelType == "AwayFromOrigin") {

        s.spawnVelocity = Spawner::SpawnVel::AwayFromOrigin;
    } else if (spawnVelType == "UseList") {

        s.spawnVelocity = Spawner::SpawnVel::UseList;

        sol::table pointsTable = tbl["spawnVel"]["points"];
        for (auto it = pointsTable.begin(); it != pointsTable.end(); ++it) {
            s.velocityList.push_back((*it).second.as<float>());
        }
    }

    s.spawnAcceleration = Spawner::SpawnAccel::Default;
    std::string spawnAccelType = tbl["spawnAccelType"];
    if (spawnAccelType == "Forward") {

        s.spawnAcceleration = Spawner::SpawnAccel::Forward;
    } else if (spawnAccelType == "Backward") {

        s.spawnAcceleration = Spawner::SpawnAccel::Backward;
    } else if (spawnAccelType == "Decel") {

        s.spawnAcceleration = Spawner::SpawnAccel::Decel;
    }


    sol::object extraComponents = tbl["extraComponents"];

    if (extraComponents != sol::nil ) {

        for (auto& cmpTable : (sol::table)extraComponents) {
            s.addComponents.emplace_back();
            for (auto& cmpDataPair : cmpTable.second.as<sol::table>()) {
                    s.addComponents.back().emplace_back(cmpDataPair.second);


            }
        }
    }

    return s;
}

template<>
Spawner buildFromLua<Spawner>(sol::object& obj) {
    Spawner s;
    s.currentBurst = 0;
    sol::table tbl = obj;
    sol::table bursts = tbl["bursts"];
    s.initialDelay = tbl["initialDelay"];
    s.totalRunCount = tbl["totalRuns"];
    /*std::cout << s.totalRunCount << '\t' << s.currentBurst << '\n';*/
    for (auto burst : bursts) {
        s.bursts.emplace_back(buildFromLua<Spawner::Burst>(burst.second));
    }
    /*std::cout << s.totalRunCount << '\t' << s.currentBurst << '\t' << s.bursts.size() << '\n';*/

    s.currentTime = s.bursts[0].repeatRate-s.initialDelay;
    return s;
}
