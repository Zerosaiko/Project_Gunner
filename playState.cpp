#include "playState.h"
#include "SDL_image.h"
#include "SDL.h"
#include "scriptcomponent.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 6000}, renderSys(&manager, 10000, window),
        scriptSys(&manager, 1), mInpSys(&manager, 2), boundsSys(&manager, 7500), colSys{&manager, 0},
        spawnSys(&manager, 1), posSyncSys(&manager, 5500), delaySys(&manager, 1), pauseSys(&manager, 1) {

    auto id = manager.createEntity();
    manager.groupManager.groupEntity("player", id);
    manager.addComponent(std::string("component:position 180, 360"), id);
    manager.addComponent(std::string("component:velocity 0, 0"), id);
    manager.addComponent(std::string("component:sprite NamelessSheet 0, 0"), id);
    manager.addComponent(std::string("component:player 80 35"), id);
    manager.addComponent(std::string("component:orientation angle 90"), id);
    manager.addComponent(std::string("component:bounds block block 0 0 360 480"), id);
    manager.addComponent(std::string("component:collider Player Point 0 0"), id);

    id = manager.createEntity();
    manager.addComponent(std::string("component:position 180, 80"), id);
    manager.addComponent(std::string("component:velocity 20, 0"), id);
    manager.addComponent(std::string("component:sprite NamelessSheet 0, 50"), id);
    manager.addComponent(std::string("component:orientation angle -90"), id);
    manager.addComponent(std::string("component:bounds bounce bounce 60, 0, 300, 120"), id);
    manager.addComponent(std::string("component:collider Enemy AABB 0 0 -14 -8 14 8"), id);
    manager.addComponent(std::string("component:spawner 10 19\
        1800 3650 Source PosRad -75 0.001 20 0 90 0\
        VelSpeed 35 0.5 0 AwayFromOrigin\
        3\
        component 0 <component:sprite NamelessSheet 3, 100>\
        component 0 <component:bounds destroy destroy 0, 0, 360, 480 boundsLimit 1 0 destroy>\
        component 0 <component:collider EnemyBullet Circle 0 0 5>\
        component 1 <\
            component:spawner 2 4\
            800 250 Source PosRad 0 10 90 5 180 20\
            VelSpeed 35 0 10 AimedAwayBySource\
            2\
            component 0 <component:sprite NamelessSheet 2, 100>\
            component 0 <component:bounds bounce wrap 0, 0, 360, 480 boundsLimit 2 1 destroy>\
            component 0 <component:collider EnemyBullet Circle 0 0 5>\
            >\
        component 1 <component:sprite NamelessSheet 4, 100>\
        component 1 <component:bounds bounce destroy 0, 0, 360, 480 boundsLimit 1 0 destroy>\
        component 1 <component:collider EnemyBullet Circle 0 0 5>\
        component 1 <component:fullDelay 500>\
        component 2 <component:sprite NamelessSheet 4, 100>\
        component 2 <component:bounds destroy destroy 0, 0, 360, 480 boundsLimit 1 0 destroy>\
        component 2 <component:collider EnemyBullet Circle 0 0 5>\
        component 2 <component:fullDelay 1000>"), id);
}

void PlayState::handleInput() {

}

void PlayState::update(float rate) {
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
