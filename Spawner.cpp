#include "Spawner.h"

const std::string Spawner::name{"spawner"};

Spawner::Spawner() {}

template<>
Spawner buildFromString<Spawner>(std::vector<std::string> str, size_t pos) {
    Spawner s;

    s.runCount = buildFromString<int32_t>(str, pos);
    s.spawnsPerRun = buildFromString<uint32_t>(str, ++pos);
    s.repeatRate = buildFromString<float>(str, ++pos);
    s.currentTime = 0.f;
    s.relative = Spawner::Relative::Absolute;
    if (str[++pos] == "Source") {
        s.relative = Spawner::Relative::Source;
    } else if (str[++pos] == "Player") {
        s.relative = Spawner::Relative::Player;
    }

    s.posDirection = Spawner::XYorDirSpeed::XY;

    s.velDirection = Spawner::XYorDirSpeed::XY;

    s.accelDirection = Spawner::XYorDirSpeed::XY;

    if (str[++pos] == "PosXY") {
        s.posDirection = Spawner::XYorDirSpeed::XY;
        s.position.xyVec.x = buildFromString<float>(str, ++pos);
        s.position.xyVec.y = buildFromString<float>(str, ++pos);
        s.position.xyVec.dx = buildFromString<float>(str, ++pos);
        s.position.xyVec.dy = buildFromString<float>(str, ++pos);
    } else if (str[pos] == "PosDirectionSpeed") {
        s.posDirection = Spawner::XYorDirSpeed::DirSpeed;
        s.position.dirSpd.direction = buildFromString<float>(str, ++pos);
        s.position.dirSpd.deltaDirection = buildFromString<float>(str, ++pos);
        s.position.dirSpd.speed = buildFromString<float>(str, ++pos);
        s.position.dirSpd.dSpeed = buildFromString<float>(str, ++pos);
    }

    if (str[++pos] == "VelXY") {
        s.velDirection = Spawner::XYorDirSpeed::XY;
        s.velocity.xyVec.x = buildFromString<float>(str, ++pos);
        s.velocity.xyVec.y = buildFromString<float>(str, ++pos);
        s.velocity.xyVec.dx = buildFromString<float>(str, ++pos);
        s.velocity.xyVec.dy = buildFromString<float>(str, ++pos);
    } else if (str[pos] == "VelDirectionSpeed") {
        s.velDirection = Spawner::XYorDirSpeed::DirSpeed;
        s.velocity.dirSpd.direction = buildFromString<float>(str, ++pos);
        s.velocity.dirSpd.deltaDirection = buildFromString<float>(str, ++pos);
        s.velocity.dirSpd.speed = buildFromString<float>(str, ++pos);
        s.velocity.dirSpd.dSpeed = buildFromString<float>(str, ++pos);
    }

    if (str[++pos] == "AccelXY") {
        s.accelDirection = Spawner::XYorDirSpeed::XY;
        s.acceleration.xyVec.x = buildFromString<float>(str, ++pos);
        s.acceleration.xyVec.y = buildFromString<float>(str, ++pos);
        s.acceleration.xyVec.dx = buildFromString<float>(str, ++pos);
        s.acceleration.xyVec.dy = buildFromString<float>(str, pos);
    } else if (str[pos] == "AccelDirectionSpeed") {
        s.accelDirection = Spawner::XYorDirSpeed::DirSpeed;
        s.acceleration.dirSpd.direction = buildFromString<float>(str, ++pos);
        s.acceleration.dirSpd.deltaDirection = buildFromString<float>(str, ++pos);
        s.acceleration.dirSpd.speed = buildFromString<float>(str, ++pos);
        s.acceleration.dirSpd.dSpeed = buildFromString<float>(str, ++pos);
    }

    s.spawnPosition = Spawner::SpawnPos::Default;
    if (str[++pos] == "AlongRectangle") {
        if (str[++pos] == "Clockwise") {
            s.spawnPosition = Spawner::SpawnPos::AlongRectangleClockwise;
        } else if (str[pos] == "CounterClockwise") {
            s.spawnPosition = Spawner::SpawnPos::AlongRectangleCounterClockwise;
        }
        s.spawnRect.minX = buildFromString<float>(str, ++pos);
        s.spawnRect.minY = buildFromString<float>(str, ++pos);
        s.spawnRect.maxX = buildFromString<float>(str, ++pos);
        s.spawnRect.maxY = buildFromString<float>(str, ++pos);
    } else if (str[pos] == "AlongArc") {
        if (str[++pos] == "Clockwise") {
            s.spawnPosition = Spawner::SpawnPos::AlongArcClockwise;
        } else if (str[pos] == "CounterClockwise") {
            s.spawnPosition = Spawner::SpawnPos::AlongArcCounterClockwise;
        }
        s.spawnArc.radius = buildFromString<float>(str, ++pos);
        s.spawnArc.minAngle = buildFromString<float>(str, ++pos);
        s.spawnArc.maxAngle = buildFromString<float>(str, ++pos);
    } else if (str[pos] == "AtPoints") {
        uint32_t pointCount = buildFromString<uint32_t>(str,++pos);
        s.spawnPoints.reserve(pointCount * 2);
        for(uint32_t i = 0; i < pointCount; ++i) {
            s.spawnPoints.push_back(buildFromString<float>(str, ++pos));
            s.spawnPoints.push_back(buildFromString<float>(str, ++pos));
        }
    }

    s.spawnVelocity = Spawner::SpawnVel::Default;
    if (str[++pos] == "Aimed") {
        s.spawnVelocity = Spawner::SpawnVel::Aimed;
    } else if (str[pos] == "TowardOrigin") {
        s.spawnVelocity = Spawner::SpawnVel::TowardOrigin;
    } else if (str[pos] == "AwayFromOrigin") {
        s.spawnVelocity = Spawner::SpawnVel::AwayFromOrigin;
    } else if (str[pos] == "AlongRectangle") {
        if (str[++pos] == "Clockwise")
            s.spawnVelocity = Spawner::SpawnVel::AlongRectangleClockwise;
        else if (str[pos] == "CounterClockwise")
            s.spawnVelocity = Spawner::SpawnVel::AlongRectangleCounterClockwise;
    } else if (str[pos] == "AgainstRectangle") {
        s.spawnVelocity = Spawner::SpawnVel::AgainstRectangle;
    } else if (str[pos] == "TowardRectangle") {
        s.spawnVelocity = Spawner::SpawnVel::TowardRectangle;
    } else if (str[pos] == "UseList") {
        s.spawnVelocity = Spawner::SpawnVel::UseList;
    } else if (str[pos] == "TowardRectangle") {
        s.spawnVelocity = Spawner::SpawnVel::TowardRectangle;
    }

    s.spawnAcceleration = Spawner::SpawnAccel::Default;
    if (str[++pos] == "Forward") {
        s.spawnAcceleration = Spawner::SpawnAccel::Forward;
    } else if (str[pos] == "Backward") {
        s.spawnAcceleration = Spawner::SpawnAccel::Backward;
    } else if (str[pos] == "Decel") {
        s.spawnAcceleration = Spawner::SpawnAccel::Decel;
    } else if (str[pos] == "Toward") {
        s.spawnAcceleration = Spawner::SpawnAccel::Toward;
    }

    while (str[++pos] == "components") {
        uint32_t nesting = 1;
        std::string pass;
        pass.reserve(300);
        while (pos < str.size() && nesting > 0) {
            if (str[++pos] == "components") ++nesting;
            else if (str[pos] == "end_components") --nesting;
            else if (nesting > 1) {
                pass += str[pos];
                pass += "\n";
            }
        }
        s.addComponents.push_back(pass);
        pass.clear();
    }

    return s;
}
