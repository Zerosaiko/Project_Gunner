#include "Message.h"

Message::Message(Type type) : type(type) {

}

Message::~Message() {

}

CollisionMessage::CollisionMessage(Type type, uint32_t col1ID, uint32_t col2ID) : Message(type),
    col1ID(col1ID), col2ID(col2ID) {
}

PlayerHit::PlayerHit(uint32_t enemyID, uint32_t playerID) : CollisionMessage(Message::Type::PlayerHit, enemyID, playerID) {}

PlayerPickup::PlayerPickup(uint32_t pickupID, uint32_t playerID) : CollisionMessage(Message::Type::EnemyHit, pickupID, playerID) {}

EnemyHit::EnemyHit(uint32_t bulletID, uint32_t enemyID) : CollisionMessage(Message::Type::EnemyHit, bulletID, enemyID) {}
