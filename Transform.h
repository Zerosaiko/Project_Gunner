#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "SDL.h"
#include <string>
#include <array>

namespace cmpName {
    extern const std::string worldTF;
    extern const std::string localTF;
}

struct TransformState {

    TransformState();

    TransformState(float originX, float originY, float angle, float scaleX, float scaleY, bool flipX, bool flipY);

    std::array<float, 16> matrix;

    float originX;

    float originY;

    float angle;

    float scaleX;

    float scaleY;

    bool flipX, flipY;

    void setOrigin(float x, float y);

    void setAngle(float angle);

    void rotate(float angle);

    void setScale(float scaleX, float scaleY);

    void scale(float scaleX, float scaleY);

    void setFlipX(bool flip);

    void setFlipY(bool flip);

    void flipXAxis();

    void flipYAxis();

};

struct WorldTransform{

    WorldTransform();

    WorldTransform(float originX, float originY, float angle, float scaleX, float scaleY, bool flipX, bool flipY);

    bool dirty;

    TransformState present;

    TransformState past;

};

struct LocalTransform {
    LocalTransform();

    LocalTransform(float originX, float originY, float angle, float scaleX, float scaleY, bool flipX, bool flipY, uint32_t parent);

    bool dirty;

    uint32_t parentTFEntity;

    TransformState state;

};

#endif // TRANSFORM_H
