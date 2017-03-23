#ifndef SPAWNER_H_INCLUDED
#define SPAWNER_H_INCLUDED

#include "component.h"
#include <vector>
#include "sol.hpp"

struct Spawner {

    static const std::string name;

    static const std::string getName();

    Spawner();

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
        Default = 0,
        Aimed = 1,
        AwayFromPlayer = 2,
        AimedBySource = 3,
        AimedAwayBySource = 4,
        TowardOrigin = 5,
        AwayFromOrigin = 6,
        UseList = 7,
    };

    enum class SpawnAccel : size_t {
        Default = 0,
        Forward = 1,
        Backward = 2,
        Decel = 3

    };

    struct Burst {

        ~Burst();

        //int32_t totalRunCount;
        int32_t runCount;
        int32_t runs;
        uint32_t spawnsPerRun;
        float repeatRate;
        float initialDelay;
        bool rotate;

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

        std::vector< std::vector<sol::object> > addComponents;
    };

    int32_t totalRunCount;

    float initialDelay;

    float currentTime;

    size_t currentBurst;

    std::vector<Burst> bursts;
};

#endif // SPAWNER_H_INCLUDED
