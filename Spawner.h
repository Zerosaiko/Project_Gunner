#ifndef SPAWNER_H_INCLUDED
#define SPAWNER_H_INCLUDED

#include "component.h"
#include <vector>

struct Spawner {

    Spawner();

    static const std::string name;

    static const std::string getName();

    struct DeltaPoint {
        float x, y, dx, dy, persistDx, persistDy;
    };

    struct DirVector {
        float direction, speed, deltaDirection, dSpeed, persistDeltaDirection, persistDSpeed;
    };

    struct DeltaSpeed {
        float current, delta, persistDelta;
    };

    enum class Relative : size_t {
        Absolute = 0,
        Source = 1,
        Player = 2,

    };

    enum class PointStyle : size_t {
        XY = 0,
        Rad = 1,
        Speed = 2
    };

    enum class SpawnPos : size_t {
        Default = 0,
        AlongList = 1,
        AlongPath = 2

    };

    enum class SpawnVel : size_t {
        None = 0,
        Default = 1,
        Aimed = 2,
        AwayFromPlayer = 3,
        AimedBySource = 4,
        AimedAwayBySource = 5,
        TowardOrigin = 6,
        AwayFromOrigin = 7,
        UseList = 8,
    };

    enum class SpawnAccel : size_t {
        None = 0,
        Default = 1,
        Forward = 2,
        Backward = 3,
        Decel = 4

    };

    int32_t runCount;
    uint32_t spawnsPerRun;
    float repeatRate;
    float initialDelay;
    float currentTime;

    Relative relative;

    PointStyle posDirection;

    PointStyle velDirection;

    PointStyle accelDirection;

    union Position {
        DeltaPoint xyVec;
        DirVector dirSpd;
    } position;

    union Velocity {
        DeltaPoint xyVec;
        DirVector dirSpd;
        DeltaSpeed speed;
    } velocity;

    union Acceleration {
        DeltaPoint xyVec;
        DirVector dirSpd;
        DeltaSpeed speed;
    } acceleration;

    std::vector<float> spawnPoints;

    std::vector<float> velocityList;

    SpawnPos spawnPosition;

    SpawnVel spawnVelocity;

    SpawnAccel spawnAcceleration;

    uint32_t cmpTypeCount;
    std::vector<std::vector<std::string>> addComponents;

};

#endif // SPAWNER_H_INCLUDED
