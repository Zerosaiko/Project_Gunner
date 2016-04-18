#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <cstdint>

struct Message {

    enum Type : uint16_t {

        EnemyHit= 0,
        PlayerHit,
        PlayerPickup,

    };

    Type type;

    Message(Type type);

    virtual ~Message() = 0;

};

struct CollisionMessage : public Message {

    CollisionMessage(Type type, uint32_t col1ID, uint32_t col2ID);

    uint32_t col1ID;
    uint32_t col2ID;
};

struct PlayerHit : public CollisionMessage {

    PlayerHit(uint32_t enemyID, uint32_t playerID);

};

struct PlayerPickup : public CollisionMessage {

    PlayerPickup(uint32_t pickupID, uint32_t playerID);

};

struct EnemyHit : public CollisionMessage {

    EnemyHit(uint32_t bulletID, uint32_t enemyID);

};

#endif // MESSAGE_H_INCLUDED
