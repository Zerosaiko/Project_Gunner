#ifndef DISPLACE_H_INCLUDED
#define DISPLACE_H_INCLUDED

#include "component.h"

class Displace : public ComponentBase {


private:

    class DisplaceFactory : public ComponentFactory {

    public:

        std::size_t build(EntityManager* manager, size_t idx, std::string instructions);

        std::size_t build(EntityManager* manager, size_t idx, ComponentBase* cmp);

        std::size_t build(EntityManager* manager, std::string instructions);

        std::size_t build(EntityManager* manager, ComponentBase* cmp);

        void deregisterManager(EntityManager*);

        void registerComponent();

        std::vector<std::string> tokenize(std::string instructions);
    };

    typedef std::vector<Displace> cmpPool;

public:
    static std::map<EntityManager*, cmpPool> componentPools;
    static const std::string getName();
    static void registerComponent();
    static DisplaceFactory* factory;

    void build(std::vector<std::string> instructions);

    Displace() : posX(0), posY(0), velX(0), velY(0) {}
    Displace(const Displace& other) : posX(other.posX), posY(other.posY), velX(other.velX), velY(other.velY), pastPosX(posX), pastPosY(posY) {}
    Displace(const float& posX, const float& posY, const float& velX, const float& velY) : posX(posX), posY(posY), velX(velX), velY(velY), pastPosX(posX), pastPosY(posY) {}
    Displace(std::vector<std::string> instructions) : posX(0), posY(0), velX(0), velY(0), pastPosX(0), pastPosY(0) {
        build(instructions);
    }
    float posX;
    float posY;
    float velX;
    float velY;
    float pastPosX;
    float pastPosY;
};

#endif // DISPLACE_H_INCLUDED
