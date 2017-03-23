#include "Transform.h"
#include "component.h"
#include "SDL_gpu.h"

const std::string Transform::name{"transform"};

Transform::Transform() : hasParent(false), local(), worldPresent(), worldPast() {}

Transform::Transform(uint32_t parentID) : hasParent(true), parentTFEntity(parentID), local(), worldPresent(), worldPast() {}

Transform::Transform(float originX, float originY, float translateX, float translateY,
                    float angle, float scaleX, float scaleY, bool flipX, bool flipY) :
    hasParent(false), worldPresent(originX, originY, translateX, translateY, angle, scaleX, scaleY, flipX, flipY), worldPast(worldPresent) {}

TransformState::TransformState() : originX(0.0f), originY(0.0f), translateX(0.0f), translateY(0.0f), angle(0.0f), scaleX(1.0f), scaleY(1.0f), flipX(false), flipY(false) {
    GPU_MatrixIdentity(matrix.data());
}

TransformState::TransformState(float originX, float originY, float translateX, float translateY,
                    float angle, float scaleX, float scaleY, bool flipX, bool flipY) :
    TransformState() {

    setOrigin(originX, originY);
    setTranslate(translateX, translateY);
    setAngle(angle);
    setScale(scaleX, scaleY);
    setFlipX(flipX);
    setFlipY(flipY);
}

void TransformState::setTranslate(float x, float y) {
    float rot[16] = {1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1};
    float vec[] = {x - translateX, y - translateY, 0};
    GPU_MatrixRotate(rot, -angle, 0, 0, 1);
    GPU_VectorApplyMatrix(vec, rot);
    GPU_MatrixTranslate(matrix.data(), vec[0], vec[1], 0);
    translateX = x;
    translateY = y;
}

void TransformState::translate(float x, float y) {
    setTranslate(translateX + x, translateY + y);
}

void TransformState::TransformState::setOrigin(float x, float y) {
    GPU_MatrixTranslate(matrix.data(), x - originX, y - originY, 0);
    originX = x;
    originY = y;
}

void TransformState::TransformState::setAngle(float angle) {
    GPU_MatrixTranslate(matrix.data(), -originX, -originY, 0);
    GPU_MatrixRotate(matrix.data(), this->angle - angle, 0, 0, 1);
    GPU_MatrixTranslate(matrix.data(), originX, originY, 0);
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

float TransformState::xPos() {
    float rot[16] = {1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1};
    float vec[] = {originX,originY, 0};
    GPU_MatrixRotate(rot, -angle, 0, 0, 1);
    GPU_VectorApplyMatrix(vec, rot);
    return translateX + vec[0];
}

float TransformState::yPos() {
    float rot[16] = {1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1};
    float vec[] = {originX,originY, 0};
    GPU_MatrixRotate(rot, -angle, 0, 0, 1);
    GPU_VectorApplyMatrix(vec, rot);
    return translateY + vec[1];
}

std::tuple<float, float> TransformState::getPos() const {
    float rot[16] = {1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1};
    float vec[] = {originX,originY, 0};
    GPU_MatrixRotate(rot, -angle, 0, 0, 1);
    GPU_VectorApplyMatrix(vec, rot);
    return std::make_tuple(translateX + vec[0],
        translateY + vec[1]);
}

const TransformState TransformState::operator*(const TransformState& other) const {
    TransformState ret = *this;
    return ret *= other;

}

TransformState& TransformState::operator*=(const TransformState& other) {
    GPU_MultiplyAndAssign(matrix.data(), const_cast<float*>(other.matrix.data()));
    originX += other.originX;
    originY += other.originY;
    translateX += other.translateX;
    translateY += other.translateY;
    angle += other.angle;
    scaleX *= other.scaleX;
    scaleY *= other.scaleY;
    if (other.flipX) flipX = !flipX;
    if (other.flipY) flipY = !flipY;
    return *this;
}

template<>
TransformState buildFromLua<TransformState>(sol::object& obj) {
    sol::table tbl = obj;
    float angle = tbl["angle"].get_or(0.0f),
    originX = tbl["origin"]["x"].get_or(0.0f),
    originY = tbl["origin"]["y"].get_or(0.0f),
    scaleX = tbl["scale"]["x"].get_or(1.0f),
    scaleY = tbl["scale"]["y"].get_or(1.0f),
    translateX = tbl["translate"]["x"].get_or(0.0f),
    translateY = tbl["translate"]["y"].get_or(0.0f);

    bool flipX = tbl["flip"]["x"].get_or(false),
    flipY = tbl["flip"]["y"].get_or(false);

    return TransformState(originX, originY, translateX, translateY, angle, scaleX, scaleY, flipX, flipY);
}

template<>
Transform buildFromLua<Transform>(sol::object& obj) {

    sol::table tbl = obj;
    Transform transform;
    sol::optional<uint32_t> parent = tbl["parent"];
    if (parent != sol::nullopt) {
        transform.hasParent = true;
        transform.parentTFEntity = tbl["parent"];
    }

    sol::object tf = tbl["transform"];
    transform.local = transform.worldPast = transform.worldPresent = buildFromLua<TransformState>(tf);
    return transform;
}

void TransformTree::setDirty(uint32_t id) {
    auto it = transforms.find(id);
    if (it != transforms.end() && !it->second.dirty) {
        it->second.dirty = true;
        dirtyList.insert(id);
        for (auto childID : it->second.children) {
            setDirty(childID);
        }
    }
}

void TransformTree::clearDirty(uint32_t id) {
    auto it = transforms.find(id);
    if (it != transforms.end() && it->second.dirty) {
        it->second.dirty = false;
        dirtyList.erase(id);
        for (auto childID : it->second.children) {
            clearDirty(childID);
        }
    }
}

TransformTree::Node::Node() : Node(0) {}

TransformTree::Node::Node(uint32_t parent) : dirty(false), parent(parent), height(0) {}

