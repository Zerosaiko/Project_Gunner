#include "collider.h"
#include <cmath>
static const float PI = acosf(-1.0f);

void OBoundingBox::setAngle(float angle) {
    angle *= PI / 180.0f;
    auto cosAng = cos(this->angle - angle);
    auto sinAng = sin(this->angle - angle);
    for (auto &vertex : vertices) {
        auto x = vertex.x - pivot.x, y = vertex.y - pivot.y;
        vertex.x = x * cosAng + y * sinAng + pivot.x;
        vertex.y = y * sinAng - x * cosAng + pivot.y;
    }
    this->angle = angle;
}

void OBoundingBox::rotate(float angle) {
    setAngle(this->angle + angle);
}

const std::string Collider::name{"collider"};

Collider::Collider() : colliderType(Collider::ColliderType::NoType) {}

template<>
Collider buildFromLua<Collider>(sol::object& obj) {
    Collider c;

    sol::table tbl = obj;

    c.collisionGroup = tbl["group"];

    c.position.x = c.position.y = 0.f;
    c.offset.x = c.offset.y = 0.f;
    std::string collidType = tbl["collider"]["type"];
    if (collidType == "Point") {
        c.colliderType = Collider::ColliderType::Point;
        c.offset.x = c.spatialBox.minX = c.spatialBox.maxX = tbl["collider"]["x"];
        c.offset.y = c.spatialBox.minY = c.spatialBox.maxY = tbl["collider"]["y"];
    } else if (collidType == "AABB") {
        c.colliderType = Collider::ColliderType::AABB;
        c.offset.x = tbl["collider"]["x"].get_or(0.0f);
        c.offset.y = tbl["collider"]["y"].get_or(0.0f);
        c.aabb.minX = tbl["collider"]["minX"].get_or(0.0f);
        c.aabb.minY = tbl["collider"]["minY"].get_or(0.0f);
        c.aabb.maxX = tbl["collider"]["maxX"].get_or(0.0f);
        c.aabb.maxY = tbl["collider"]["maxY"].get_or(0.0f);
        c.spatialBox = c.aabb;
        c.spatialBox.minX += c.offset.x;
        c.spatialBox.maxX += c.offset.x;
        c.spatialBox.minY += c.offset.y;
        c.spatialBox.maxY += c.offset.y;
    } else if (collidType == "Circle") {
        c.colliderType = Collider::ColliderType::Circle;
        c.offset.x = tbl["collider"]["x"].get_or(0.0f);
        c.offset.y = tbl["collider"]["y"].get_or(0.0f);
        c.circle.radius = tbl["collider"]["radius"].get_or(0.0f);
        c.spatialBox.minX = c.offset.x - c.circle.radius;
        c.spatialBox.maxX = c.offset.x + c.circle.radius;
        c.spatialBox.minY = c.offset.y - c.circle.radius;
        c.spatialBox.maxY = c.offset.y + c.circle.radius;
    } else if (collidType == "OBB") {
        c.colliderType = Collider::ColliderType::OBB;
        c.offset.x = tbl["collider"]["x"].get_or(0.0f);
        c.offset.y = tbl["collider"]["y"].get_or(0.0f);
        c.obb.pivot.x = tbl["pivot"]["x"].get_or(0.0f);
        c.obb.pivot.y = tbl["pivot"]["y"].get_or(0.0f);
        c.obb.angle = 0.0f;
        int j = 1;
        c.spatialBox.minX = c.spatialBox.maxX = tbl["collider"]["vertices"][j]["x"].get_or(0.0f);
        c.spatialBox.minY = c.spatialBox.maxY = tbl["collider"]["vertices"][j]["y"].get_or(0.0f);
        for (uint8_t i = 0; i < 4; i++) {
            decltype(i) j = i + 1;
            c.obb.vertices[i].x = tbl["collider"]["vertices"][j]["x"].get_or(0.0f);
            c.obb.vertices[i].y = tbl["collider"]["vertices"][j]["y"].get_or(0.0f);
            c.spatialBox.minX = std::min(c.spatialBox.minX, c.obb.vertices[i].x);
            c.spatialBox.maxX = std::max(c.spatialBox.maxX, c.obb.vertices[i].x);
            c.spatialBox.minY = std::min(c.spatialBox.minY, c.obb.vertices[i].y);
            c.spatialBox.maxY = std::max(c.spatialBox.maxY, c.obb.vertices[i].y);
        }
    }

    c.collisionHandlers = tbl["handlers"];

    return c;
}
