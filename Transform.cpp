#include "Transform.h"
#include "component.h"
#include "SDL_gpu.h"

const std::string Transform::name{"transform"};

Transform::Transform() : dirty(true), hasParent(false), local(), worldPresent(), worldPast() {}

Transform::Transform(uint32_t parentID) : dirty(true), hasParent(true), parentTFEntity(parentID), local(), worldPresent(), worldPast() {}

Transform::Transform(float originX, float originY, float translateX, float translateY,
                    float angle, float scaleX, float scaleY, bool flipX, bool flipY) :
    dirty(true), hasParent(false), worldPresent(originX, originY, translateX, translateY, angle, scaleX, scaleY, flipX, flipY), worldPast(worldPresent) {}

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
    float vec[] = {x - translateX + originX, y - translateY + originY, 0};
    GPU_MatrixRotate(rot, -angle, originX, originY, 1);
    GPU_VectorApplyMatrix(vec, rot);
    GPU_MatrixTranslate(matrix.data(), vec[0], vec[1], 0);

    translateX = x;
    translateY = y;
}

void TransformState::translate(float x, float y) {
    setTranslate(translateX + x, translateY + y);
}

void TransformState::TransformState::setOrigin(float x, float y) {
    originX = x;
    originY = y;;
}

void TransformState::TransformState::setAngle(float angle) {
    GPU_MatrixTranslate(matrix.data(), originX, originY, 0);
    GPU_MatrixRotate(matrix.data(), this->angle - angle, 0, 0, 1);
    GPU_MatrixTranslate(matrix.data(), -originX, -originY, 0);
    this->angle = angle;
}

void TransformState::TransformState::rotate(float angle) {
    setAngle(this->angle + angle);
}

void TransformState::TransformState::setScale(float scaleX, float scaleY) {
    GPU_MatrixTranslate(matrix.data(), originX, originY, 0);
    GPU_MatrixScale(matrix.data(), scaleX / this->scaleX, scaleY / this->scaleY, 1);
    GPU_MatrixTranslate(matrix.data(), -originX, -originY, 0);
    this->scaleX = scaleX;
    this->scaleY = scaleY;
}

void TransformState::TransformState::scale(float scaleX, float scaleY) {
    setScale(this->scaleX * scaleX, this->scaleY * scaleY);
}

void TransformState::TransformState::setFlipX(bool flipX) {
    GPU_MatrixTranslate(matrix.data(), originX, originY, 0);
    GPU_MatrixScale(matrix.data(), 1 - 2 * flipX, 1, 1);
    GPU_MatrixTranslate(matrix.data(), -originX, -originY, 0);
    this->flipX = flipX;
}

void TransformState::TransformState::setFlipY(bool flipY) {
    GPU_MatrixTranslate(matrix.data(), originX, originY, 0);
    GPU_MatrixScale(matrix.data(), 1, (1 - 2 * flipY), 1);
    GPU_MatrixTranslate(matrix.data(), -originX, -originY, 0);
    this->flipY = flipY;
}

void TransformState::TransformState::flipXAxis() {
    setFlipX(!flipX);

}

void TransformState::TransformState::flipYAxis() {
    setFlipY(!flipY);
}

const TransformState TransformState::operator*(const TransformState& other) const {
    TransformState ret = *this;
    return ret *= other;

}

TransformState& TransformState::operator*=(const TransformState& other) {
    GPU_MultiplyAndAssign(matrix.data(), const_cast<float*>(other.matrix.data()));
    originX += other.originX;
    originY += other.originY;
    if (other.flipX) flipX = !flipX;
    if (other.flipY) flipY = !flipY;
    angle += other.angle;
    scaleX *= other.scaleX;
    scaleY *= other.scaleY;
    translateX += other.translateX;
    translateY += other.translateY;
    return *this;
}

template<>
TransformState buildFromString<TransformState>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {
    float angle = 0.0f, originX = 0.0f, originY = 0.0f, scaleX = 1.0f, scaleY = 1.0f, translateX = 0.0f, translateY = 0.0f;
    bool flipX = false, flipY = false;
    uint32_t count = buildFromString<uint32_t>(str, pos);

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
        } else if (str[pos] == "position") {
            translateX = buildFromString<float>(str, ++pos);
            translateY = buildFromString<float>(str, pos);
        }
    }

    return TransformState(originX, originY, translateX, translateY, angle, scaleX, scaleY, flipX, flipY);
}

template<>
Transform buildFromString<Transform>(std::vector<std::string>& str, std::vector<std::string>::size_type& pos) {

    Transform transform;
    if (buildFromString<uint32_t>(str, pos)) {
        transform.hasParent = true;
        transform.parentTFEntity = buildFromString<uint32_t>(str, pos);
    }

    transform.local = transform.worldPast = transform.worldPresent = buildFromString<TransformState>(str, pos);
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
            setDirty(childID);
        }
    }
}

TransformTree::Node::Node() : dirty(true), parent(0) {}

TransformTree::Node::Node(bool dirty, uint32_t parent) : dirty(dirty), parent(parent){}

