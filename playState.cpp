#include "playState.h"
#include "SDL.h"
#include "scriptcomponent.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 6000}, renderSys(&manager, 10000, window),
        mInpSys(&manager, 3), boundsSys(&manager, 7500), colSys{&manager, 0}, spawnSys(&manager, 2),
        posSyncSys(&manager, 5500), delaySys(&manager, 1), pauseSys(&manager, 1), playSys{&manager, 0},
        animSys(&manager, 8500), shieldSys(&manager, 9000), lifeTimerSys(&manager, 1) {

    auto id = manager.createEntity();
    manager.addComponent(std::string("component:position 180, 360"), id);
    manager.addComponent(std::string("component:velocity 0, 0"), id);
    manager.addComponent(std::string("component:sprite NamelessSheet 0, 0"), id);
    manager.addComponent(std::string("component:player 80 35"), id);
    manager.addComponent(std::string("component:worldTransform 1 angle 90"), id);
    manager.addComponent(std::string("component:bounds block block 0 0 360 480"), id);
    manager.addComponent(std::string("component:collider Player Point 0 0"), id);
    manager.addComponent(std::string("component:shield 500"), id);

}

void PlayState::handleInput() {

}

void PlayState::update(float rate) {
    int32_t keyNum = 0;
    uint8_t const * const keys = SDL_GetKeyboardState(&keyNum);

    static float cd = 1000.f;

    cd -= rate * 1000.0f;

    if (keys[SDL_SCANCODE_1] && cd <= 0) {
        auto id = manager.createEntity();
        manager.addComponent(std::string("component:position 180, 80"), id);
        manager.addComponent(std::string("component:velocity 20, 0"), id);
        manager.addComponent(std::string("component:sprite NamelessSheet 0, 50"), id);
        manager.addComponent(std::string("component:worldTransform 2 angle -90 scale 2 2"), id);
        manager.addComponent(std::string("component:bounds bounce bounce 60, 0, 300, 120"), id);
        manager.addComponent(std::string("component:collider Enemy AABB 0 0 -14 -8 14 8"), id);
        manager.addComponent(std::string("component:spawner 20 20\
            1800 3650 Source PosRad -75 0.001 185 0 120 0\
            VelSpeed 35 0.25 0 AwayFromOrigin\
            3\
            component 0 <component:sprite NamelessSheet 3, 100>\
            component 0 <component:bounds destroy destroy 0, 0, 360, 480 boundsLimit 1 0 destroy>\
            component 0 <component:collider EnemyBullet Circle 0 0 5>\
            component 1 <component:sprite NamelessSheet 4, 100>\
            component 1 <component:bounds bounce wrap 0, 0, 360, 480 boundsLimit 2 1 destroy>\
            component 1 <component:collider EnemyBullet Circle 0 0 5>\
            component 1 <component:fullDelay 400>\
            component 2 <component:sprite NamelessSheet 3, 100>\
            component 2 <component:bounds destroy destroy 0, 0, 360, 480 boundsLimit 1 0 destroy>\
            component 2 <component:collider EnemyBullet Circle 0 0 5>\
            component 2 <component:fullDelay 800>"), id);
        cd = 1000.f;
    } else if (keys[SDL_SCANCODE_2] && cd <= 0) {
        auto id = manager.createEntity();
        manager.addComponent(std::string("component:position 0, 0"), id);
        manager.addComponent(std::string("component:spawner 30 6\
            900 2400 Source PosRad 270 0.001 5 5 15 0\
            VelSpeed 30 -0.25 0 AwayFromOrigin\
            3\
            component 0 <component:sprite NamelessSheet 4, 100>\
            component 0 <component:bounds bounce destroy 0, 0, 360, 480 boundsLimit 2 1 destroy>\
            component 0 <component:collider EnemyBullet Circle 0 0 5>\
            component 1 <component:sprite NamelessSheet 4, 100>\
            component 1 <component:bounds bounce destroy 0, 0, 360, 480 boundsLimit 2 1 destroy>\
            component 1 <component:collider EnemyBullet Circle 0 0 5>\
            component 1 <component:fullDelay 250>\
            component 2 <component:sprite NamelessSheet 4, 100>\
            component 2 <component:bounds bounce destroy 0, 0, 360, 480 boundsLimit 2 1 destroy>\
            component 2 <component:collider EnemyBullet Circle 0 0 5>\
            component 2 <component:fullDelay 500>"), id);
        id = manager.createEntity();
        manager.addComponent(std::string("component:position 360, 0"), id);
        manager.addComponent(std::string("component:spawner 30 6\
            900 1600 Source PosRad 270 0.001 -5 5 -15 0\
            VelSpeed 30 -0.25 0 AwayFromOrigin\
            3\
            component 0 <component:sprite NamelessSheet 4, 100>\
            component 0 <component:bounds bounce destroy 0, 0, 360, 480 boundsLimit 2 1 destroy>\
            component 0 <component:collider EnemyBullet Circle 0 0 5>\
            component 1 <component:sprite NamelessSheet 4, 100>\
            component 1 <component:bounds bounce destroy 0, 0, 360, 480 boundsLimit 2 1 destroy>\
            component 1 <component:collider EnemyBullet Circle 0 0 5>\
            component 1 <component:fullDelay 250>\
            component 2 <component:sprite NamelessSheet 4, 100>\
            component 2 <component:bounds bounce destroy 0, 0, 360, 480 boundsLimit 2 1 destroy>\
            component 2 <component:collider EnemyBullet Circle 0 0 5>\
            component 2 <component:fullDelay 500>"), id);
        cd = 1000.f;
    } else if (keys[SDL_SCANCODE_3] && cd <= 0) {
        auto id = manager.createEntity();
        manager.addComponent(std::string("component:position 180, 240"), id);
        manager.addComponent(std::string("component:sprite NamelessSheet 0, 50"), id);
        manager.addComponent(std::string("component:worldTransform 2 angle -90 scale 2 2"), id);
        manager.addComponent(std::string("component:collider Enemy AABB 0 0 -14 -8 14 8"), id);
        manager.addComponent(std::string("component:spawner 30 24\
            1000 1400 Source PosRad 0 0.001 215 0 195 0\
            VelSpeed 20 0.75 0 AwayFromOrigin\
            3\
            component 0 <component:sprite NamelessSheet 2, 100>\
            component 0 <component:bounds wrap destroy 0, 0, 360, 480 boundsLimit 1 1 destroy>\
            component 0 <component:collider EnemyBullet Circle 0 0 5>\
            component 1 <component:sprite NamelessSheet 3, 100>\
            component 1 <component:bounds wrap destroy 0, 0, 360, 480 boundsLimit 1 1 destroy>\
            component 1 <component:collider EnemyBullet Circle 0 0 5>\
            component 1 <component:fullDelay 250>\
            component 2 <component:sprite NamelessSheet 4, 100>\
            component 2 <component:bounds wrap destroy 0, 0, 360, 480 boundsLimit 1 1 destroy>\
            component 2 <component:collider EnemyBullet Circle 0 0 5>\
            component 2 <component:fullDelay 500>"), id);
        cd = 1000.f;
    }

    auto beg = SDL_GetPerformanceCounter();
    manager.update(rate);
    auto ed = SDL_GetPerformanceCounter();
    //std::cout << "UPDATE - " << ((ed - beg) * 1000.f / SDL_GetPerformanceFrequency()) << std::endl;

}

void PlayState::render(float remainder) {
    renderSys.render(remainder);
}

PlayState::~PlayState() {
    window = nullptr;
}
