#include "playState.h"
#include "SDL_image.h"
#include "scriptcomponent.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 6000}, renderSys{&manager, 10000, window},
        scriptSys(&manager, 0), mInpSys(&manager, 1), boundsSys(&manager, 7500), colSys{&manager, 5999},
        spawnSys(&manager, 0), posSyncSys(&manager, 5500), delaySys(&manager, 0) {
    manager.addSystem(&moveSys);
    manager.addSystem(&renderSys);
    manager.addSystem(&scriptSys);
    manager.addSystem(&mInpSys);
    manager.addSystem(&boundsSys);
    manager.addSystem(&colSys);
    manager.addSystem(&spawnSys);
    manager.addSystem(&posSyncSys);
    manager.addSystem(&delaySys);

    manager.createEntity(0);
    manager.tagManager.tagEntity("player", 0);
    manager.addComponent(std::string("component:script 2000 \
        <@onStart\
        \ncreate %self component:position 160, 360\
        \nstop_\ncreate %self\ncomponent:velocity 0, 0\
        \nstop_\ncreate %self\ncomponent:sprite NamelessSheet 0, 0\
        \nstop_\ncreate %self component:speed 80 \
        \nstop_\ncreate %self component:focusSpeed 35 \
        \nstop_\ncreate %self component:bounds block 0 0 320 480 \
        \nstop_\ncreate %self component:collider Player Point 0 0 \
        \nstop_ @onUpdate\n end_script>"), 0);

        manager.addComponent(std::string("component:position 240, 240"), 1);
        manager.addComponent(std::string("component:spawner -1 3\
            360 400 Source PosRad 0 1 90 0 6 0 \
            VelSpeed 60 -8 0 AwayFromOrigin \
            3\
            component 0 <component:sprite NamelessSheet 2, 0>\
            component 0 <component:bounds destroy 0 0 320 480>\
            component 0 <component:collider Enemy Circle 0 0 5>\
            component 1 <component:sprite NamelessSheet 3, 0>\
            component 1 <component:bounds destroy 0 0 320 480>\
            component 1 <component:collider Enemy Circle 0 0 5>\
            component 1 <component:fullDelay 90>\
            component 2 <component:sprite NamelessSheet 2, 0>\
            component 2 <component:bounds destroy 0 0 320 480>\
            component 2 <component:collider Enemy Circle 0 0 5>\
            component 2 <component:fullDelay 225>"), 1);

        manager.addComponent(std::string("component:position 80, 240"), 2);
        manager.addComponent(std::string("component:spawner -1 3\
            360 200 Source PosRad 90 1 90 0 6 0 \
            VelSpeed 36 8 0 AwayFromOrigin \
            3\
            component 0 <component:sprite NamelessSheet 2, 0>\
            component 0 <component:bounds destroy 0 0 320 480>\
            component 0 <component:collider Enemy Circle 0 0 5>\
            component 1 <component:sprite NamelessSheet 3, 0>\
            component 1 <component:bounds destroy 0 0 320 480>\
            component 1 <component:collider Enemy Circle 0 0 5>\
            component 1 <component:fullDelay 90>\
            component 2 <component:sprite NamelessSheet 2, 0>\
            component 2 <component:bounds destroy 0 0 320 480>\
            component 2 <component:collider Enemy Circle 0 0 5>\
            component 2 <component:fullDelay 225>"), 2);

        manager.addComponent(std::string("component:position 160, 160"), 3);
        manager.addComponent(std::string("component:spawner -1 50\
            800 400 Source PosRad 270 -12.5 -3.6 0.5 45 0 \
            VelSpeed 60 -0.2 0 TowardOrigin \
            3\
            component 0 <component:sprite NamelessSheet 2, 0>\
            component 0 <component:bounds destroy 0 0 320 480>\
            component 0 <component:collider Enemy Circle 0 0 5>\
            component 1 <component:sprite NamelessSheet 3, 0>\
            component 1 <component:bounds destroy 0 0 320 480>\
            component 1 <component:collider Enemy Circle 0 0 5>\
            component 1 <component:fullDelay 400>\
            component 2 <component:sprite NamelessSheet 3, 0>\
            component 2 <component:bounds destroy 0 0 320 480>\
            component 2 <component:collider Enemy Circle 0 0 5>\
            component 2 <component:fullDelay 600>"), 3);

}

void PlayState::handleInput() {

}

void PlayState::update(float rate) {
    manager.update(rate);
}

void PlayState::render(float remainder) {
    renderSys.render(remainder);
}

PlayState::~PlayState() {
    window = nullptr;
}
