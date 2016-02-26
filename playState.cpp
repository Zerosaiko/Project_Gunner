#include "playState.h"
#include "SDL_image.h"
#include "SDL.h"
#include "scriptcomponent.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 6000}, renderSys(&manager, 10000, window),
        scriptSys(&manager, 0), mInpSys(&manager, 1), boundsSys(&manager, 7500), colSys{&manager, 5999},
        spawnSys(&manager, 0), posSyncSys(&manager, 5500), delaySys(&manager, 0), pauseSys(&manager, 0) {

    auto id = manager.createEntity();
    manager.tagManager.tagEntity("player", id);
    manager.addComponent(std::string("component:script 2000 \
        <@onStart\
        \ncreate %self component:position 160, 360\
        \nstop_\ncreate %self component:velocity 0, 0\
        \nstop_\ncreate %self component:sprite NamelessSheet 0, 0\
        \nstop_\ncreate %self component:speed 80 \
        \nstop_\ncreate %self component:focusSpeed 35 \
        \nstop_\ncreate %self component:bounds block 0 0 320 480 \
        \nstop_\ncreate %self component:collider Player Point 0 0 \
        \nstop_ @onUpdate\n end_script>"), id);

    id = manager.createEntity();
    manager.addComponent(std::string("component:script 2000 \
        <@onStart\
        \ncreate %self component:position 160, 360\
        \nstop_\ncreate %self component:velocity 0, 0\
        \nstop_\ncreate %self component:sprite NamelessSheet 0, 0\
        \nstop_\ncreate %self component:speed 80 \
        \nstop_\ncreate %self component:focusSpeed 35 \
        \nstop_\ncreate %self component:bounds block 0 0 320 480 \
        \nstop_\ncreate %self component:collider Player Point 0 0 \
        \nstop_ @onUpdate\n end_script>"), id);

    id = manager.createEntity();
    manager.addComponent(std::string("component:position 80, 80"), id);
    manager.addComponent(std::string("component:spawner -1 250\
        2000 0 Source PosRad 0 10 1.8 0 12 0 \
        VelSpeed 100 0 0 TowardOrigin \
        4\
        component 0 <component:bounds destroy 0 0 320 480>\
        component 0 <component:collider Enemy Circle 0 0 5>\
        component 0 <component:sprite NamelessSheet 3, 0>\
        component 1 <component:bounds destroy 0 0 320 480>\
        component 1 <component:collider Enemy Circle 0 0 5>\
        component 1 <component:sprite NamelessSheet 2, 0>\
        component 1 <component:pauseDelay 500>\
        component 2 <component:bounds destroy 0 0 320 480>\
        component 2 <component:collider Enemy Circle 0 0 5>\
        component 2 <component:sprite NamelessSheet 3, 0>\
        component 2 <component:pauseDelay 1000>\
        component 3 <component:bounds destroy 0 0 320 480>\
        component 3 <component:collider Enemy Circle 0 0 5>\
        component 3 <component:sprite NamelessSheet 2, 0>\
        component 3 <component:pauseDelay 1500>"), id);

    id = manager.createEntity();
    manager.addComponent(std::string("component:position 240, 80"), id);
    manager.addComponent(std::string("component:spawner -1 250\
        2000 750 Source PosRad 0 10 5.76 0 -12 0 \
        VelSpeed 100 0 0 TowardOrigin \
        4\
        component 0 <component:bounds destroy 0 0 320 480>\
        component 0 <component:collider Enemy Circle 0 0 5>\
        component 0 <component:sprite NamelessSheet 3, 0>\
        component 1 <component:bounds destroy 0 0 320 480>\
        component 1 <component:collider Enemy Circle 0 0 5>\
        component 1 <component:sprite NamelessSheet 2, 0>\
        component 1 <component:pauseDelay 500>\
        component 2 <component:bounds destroy 0 0 320 480>\
        component 2 <component:collider Enemy Circle 0 0 5>\
        component 2 <component:sprite NamelessSheet 3, 0>\
        component 2 <component:pauseDelay 1000>\
        component 3 <component:bounds destroy 0 0 320 480>\
        component 3 <component:collider Enemy Circle 0 0 5>\
        component 3 <component:sprite NamelessSheet 2, 0>\
        component 3 <component:pauseDelay 1500>"), id);

}

void PlayState::handleInput() {

}

void PlayState::update(float rate) {
    auto beg = SDL_GetPerformanceCounter();
    manager.update(rate);
    auto ed = SDL_GetPerformanceCounter();
    std::cout << "UPDATE - " << ((ed - beg) * 1000.f / SDL_GetPerformanceFrequency()) << std::endl;

}

void PlayState::render(float remainder) {
    renderSys.render(remainder);
}

PlayState::~PlayState() {
    window = nullptr;
}
