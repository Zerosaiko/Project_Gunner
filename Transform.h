#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "SDL.h"
#include <string>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct TransformState {

    TransformState();

    TransformState(float originX, float originY, float translateX, float translateY,  float angle, float scaleX, float scaleY, bool flipX, bool flipY);

    std::array<float, 16> matrix;

    float originX;

    float originY;

    float translateX;

    float translateY;

    float angle;

    float scaleX;

    float scaleY;

    bool flipX, flipY;

    void setOrigin(float x, float y);

    void setTranslate(float translateX, float translateY);

    void translate(float translateX, float translateY);

    void setAngle(float angle);

    void rotate(float angle);

    void setScale(float scaleX, float scaleY);

    void scale(float scaleX, float scaleY);

    void setFlipX(bool flip);

    void setFlipY(bool flip);

    void flipXAxis();

    void flipYAxis();

    const TransformState operator*(const TransformState& other) const;

    TransformState& operator*=(const TransformState& other);

};

struct Transform {

    static const std::string name;

    Transform();

    Transform(uint32_t parentID);

    Transform(float originX, float originY, float translateX, float translateY, float angle, float scaleX, float scaleY, bool flipX, bool flipY);

    Transform(float originX, float originY, float translateX, float translateY, float angle, float scaleX, float scaleY, bool flipX, bool flipY, uint32_t parentID);

    bool dirty;

    bool hasParent;

    uint32_t parentTFEntity;

    TransformState local;

    TransformState worldPresent;

    TransformState worldPast;

};

struct TransformTree {

    struct Node {
        bool dirty;
        uint32_t parent;
        std::vector<uint32_t> children;

        Node();

        Node(bool dirty, uint32_t parent);

    };

    std::unordered_map< uint32_t, Node > transforms;

    std::unordered_set<uint32_t> dirtyList;

    void setDirty(uint32_t id);

    void clearDirty(uint32_t id);

};

#endif // TRANSFORM_H
