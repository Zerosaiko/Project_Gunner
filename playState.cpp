#include "playState.h"
#include "SDL_image.h"
#include "scriptcomponent.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 6000}, renderSys{&manager, 10000, window},
        scriptSys(&manager, 0), mInpSys(&manager, 1), boundsSys(&manager, 7500), colSys{&manager, 5999},
        spawnSys(&manager, 0), posSyncSys(&manager, 5500), delaySys(&manager, 0) {

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
    manager.addComponent(std::string("component:position 160, 160"), id);
    manager.addComponent(std::string("component:spawner -1 200\
        1000 0 Source PosRad 0 0.001 10 0 4 0 \
        VelSpeed 100 -0.001 0 AwayFromOrigin \
        2\
        component 0 <component:bounds destroy 0 0 320 480>\
        component 0 <component:collider Enemy Circle 0 0 5>\
        component 0 <component:sprite NamelessSheet 3, 0>\
        component 1 <component:bounds destroy 0 0 320 480>\
        component 1 <component:collider Enemy Circle 0 0 5>\
        component 1 <component:sprite NamelessSheet 2, 0>"), id);

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
