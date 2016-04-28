#include "Transform.h"
#include "component.h"
#include "SDL_gpu.h"

const std::string cmpName::worldTF{"worldTransform"};

const std::string cmpName::localTF{"localTransform"};

WorldTransform::WorldTransform() : dirty(true), present(), past() {}

WorldTransform::WorldTransform(float originX, float originY, float angle, float scaleX, float scaleY, bool flipX, bool flipY) :
    dirty(true), present(originX, originY, angle, scaleX, scaleY, flipX, flipY), past(present) {}

LocalTransform::LocalTransform() : dirty(true), state(), parentTFEntity(0) {}

LocalTransform::LocalTransform(float originX, float originY, float angle, float scaleX, float scaleY, bool flipX, bool flipY, uint32_t parent) :
    dirty(true), state(originX, originY, angle, scaleX, scaleY, flipX, flipY), parentTFEntity(parent) {}

TransformState::TransformState() : originX(0.0f), originY(0.0f), angle(0.0f), scaleX(1.0f), scaleY(1.0f), flipX(false), flipY(false) {
    GPU_MatrixIdentity(matrix.data());
}

TransformState::TransformState(float originX, float originY, float angle, float scaleX, float scaleY, bool flipX, bool flipY) :
    TransformState() {

    setOrigin(originX, originY);
    setAngle(angle);
    setScale(scaleX, scaleY);
    setFlipX(flipX);
    setFlipY(flipY);

}
void TransformState::TransformState::setOrigin(float x, float y) {
    originX = x;
    originY = y;
}

void TransformState::TransformState::setAngle(float angle) {
    GPU_MatrixRotate(matrix.data(), this->angle - angle, originX, originY, 1);
    this->angle = angle;
}

void TransformState::TransformState::rotate(float angle) {
    setAngle(this->angle + angle);
}

void TransformState::TransformState::setScale(float scaleX, float scaleY) {
    GPU_MatrixTranslate(matrix.data(), -originX, -originY, 0);
    GPU_MatrixScale(matrix.data(), scaleX / this->scaleX, scaleY / this->scaleY, 1);
    GPU_MatrixTranslate(matrix.data(), originX, originY, 0);
    this->scaleX = scaleX;
    this->scaleY = scaleY;
}

void TransformState::TransformState::scale(float scaleX, float scaleY) {
    setScale(this->scaleX * scaleX, this->scaleY * scaleY);
}

void TransformState::TransformState::setFlipX(bool flipX) {
    GPU_MatrixTranslate(matrix.data(), -originX, -originY, 0);
    GPU_MatrixScale(matrix.data(), 1 - 2 * flipX, 1, 1);
    GPU_MatrixTranslate(matrix.data(), originX, originY, 0);
    this->flipX = flipX;
}

void TransformState::TransformState::setFlipY(bool flipY) {
    GPU_MatrixTranslate(matrix.data(), -originX, -originY, 0);
    GPU_MatrixScale(matrix.data(), 1, (1 - 2 * flipY), 1);
    GPU_MatrixTranslate(matrix.data(), originX, originY, 0);
    this->flipY = flipY;
}

void TransformState::TransformState::flipXAxis() {
    setFlipX(!flipX);

}

void TransformState::TransformState::flipYAxis() {
    setFlipY(!flipY);
}

template<>
WorldTransform buildFromString<WorldTransform>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    float angle = 0.0f, originX = 0.0f, originY = 0.0f, scaleX = 1.0f, scaleY = 1.0f;
    bool flipX = false, flipY = false;
    uint32_t count = buildFromString<uint32_t>(str, pos);
    if (!count) count = 5;

    for(; count > 0; --count) {
        if (str[pos] == "angle") {
            angle = buildFromString<float>(str, ++pos);
        } else if (str[pos] == "flipX") {
            flipX = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "flipY") {
            flipY = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "origin") {
            originX = buildFromString<int32_t>(str, ++pos);
            originY = buildFromString<int32_t>(str, pos);
        } else if (str[pos] == "scale") {
            scaleX = buildFromString<float>(str, ++pos);
            scaleY = buildFromString<float>(str, pos);
        }
    }

    return WorldTransform(originX, originY, angle, scaleX, scaleY, flipX, flipY);
}

template<>
LocalTransform buildFromString<LocalTransform>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    float angle = 0.0f, originX = 0.0f, originY = 0.0f, scaleX = 1.0f, scaleY = 1.0f;
    bool flipX = false, flipY = false;
    uint32_t parentID = buildFromString<uint32_t>(str, pos);
    uint32_t count = buildFromString<uint32_t>(str, pos);
    if (!count) count = 5;

    for(; count > 0; --count) {
        if (str[pos] == "angle") {
            angle = buildFromString<float>(str, ++pos);
        } else if (str[pos] == "flipX") {
            flipX = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "flipY") {
            flipY = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "origin") {
            originX = buildFromString<int32_t>(str, ++pos);
            originY = buildFromString<int32_t>(str, pos);
        } else if (str[pos] == "scale") {
            scaleX = buildFromString<float>(str, ++pos);
            scaleY = buildFromString<float>(str, pos);
        }
    }

    return LocalTransform(originX, originY, angle, scaleX, scaleY, flipX, flipY, parentID);
}

template<>
TransformState buildFromString<TransformState>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    float angle = 0.0f, originX = 0.0f, originY = 0.0f, scaleX = 1.0f, scaleY = 1.0f;
    bool flipX = false, flipY = false;
    uint32_t count = buildFromString<uint32_t>(str, pos);
    if (!count) count = 5;

    for(; count > 0; --count) {
        if (str[pos] == "angle") {
            angle = buildFromString<float>(str, ++pos);
        } else if (str[pos] == "flipX") {
            flipX = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "flipY") {
            flipY = buildFromString<uint32_t>(str, ++pos);
        } else if (str[pos] == "origin") {
            originX = buildFromString<int32_t>(str, ++pos);
            originY = buildFromString<int32_t>(str, pos);
        } else if (str[pos] == "scale") {
            scaleX = buildFromString<float>(str, ++pos);
            scaleY = buildFromString<float>(str, pos);
        }
    }

    return TransformState(originX, originY, angle, scaleX, scaleY, flipX, flipY);
}
