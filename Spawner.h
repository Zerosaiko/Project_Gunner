#ifndef SPAWNER_H_INCLUDED
#define SPAWNER_H_INCLUDED

#include "component.h"
#include <vector>

struct Spawner {

    Spawner();

    static const std::string name;

    struct DeltaPoint {
        float x, y, dx, dy;
    };

    struct Rectangle {
        float minX, minY, maxX, maxY;
    };

    struct Arc {
        float radius, minAngle, maxAngle;
    };

    struct DirVector {
        float direction, deltaDirection, speed, dSpeed;
    };

    enum class Relative : size_t {
        Absolute = 0,
        Source = 1,
        Player = 2,

    };

    enum class XYorDirSpeed : size_t {
        XY = 0,
        DirSpeed = 1
    };

    enum class SpawnPos : size_t {
        Default = 0,
        AlongRectangleClockwise = 1,
        AlongRectangleCounterClockwise = 2,
        AlongArcClockwise = 3,
        AlongArcCounterClockwise = 4,
        AlongList = 5,
        AlongPath = 6

    };

    enum class SpawnVel : size_t {
        Default = 0,
        Aimed = 1,
        TowardOrigin = 2,
        AwayFromOrigin = 3,
        AlongRectangleClockwise = 4,
        AlongRectangleCounterClockwise = 5,
        AgainstRectangle = 6,
        TowardRectangle = 7,
        UseList = 8,
    };

    enum class SpawnAccel : size_t {
        Default = 0,
        Forward = 1,
        Backward = 2,
        Decel = 3,
        Toward = 4

    };

    int32_t runCount;
    uint32_t spawnsPerRun;
    float repeatRate;
    float currentTime;

    Relative relative;

    XYorDirSpeed posDirection;

    XYorDirSpeed velDirection;

    XYorDirSpeed accelDirection;

    union Position {
        DeltaPoint xyVec;
        DirVector dirSpd;
    } position;

    union Velocity {
        DeltaPoint xyVec;
        DirVector dirSpd;
        float speed;
    } velocity;

    union Acceleration {
        DeltaPoint xyVec;
        DirVector dirSpd;
        float speed;
    } acceleration;

    SpawnPos spawnPosition;

    std::vector<float> spawnPoints;

    union {
        Rectangle spawnRect;
        Arc spawnArc;
    };

    SpawnVel spawnVelocity;

    SpawnAccel spawnAcceleration;

    std::vector<std::string> addComponents;

};

#endif // SPAWNER_H_INCLUDED
