#include "playState.h"
#include "SDL_image.h"
#include "scriptcomponent.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 6000}, renderSys{&manager, 10000, window},
        scriptSys(&manager, 0), mInpSys(&manager, 1), boundsSys(&manager, 7500), colSys{&manager, 5999},
        spawnSys(&manager, 0) {
    manager.addSystem(&moveSys);
    manager.addSystem(&renderSys);
    manager.addSystem(&scriptSys);
    manager.addSystem(&mInpSys);
    manager.addSystem(&boundsSys);
    manager.addSystem(&colSys);
    manager.addSystem(&spawnSys);

    manager.createEntity(0);
    manager.tagManager.tagEntity("player", 0);
    manager.addComponent(std::string("component:script 2000 \
        <@onStart\
        \ncreate %self component:position 180, 240\
        \nstop_\ncreate %self\ncomponent:velocity 0, 0\
        \nstop_\ncreate %self\ncomponent:sprite NamelessSheet 0, 0\
        \nstop_\ncreate %self component:speed 80 \
        \nstop_\ncreate %self component:focusSpeed 35 \
        \nstop_\ncreate %self component:bounds block 0 0 320 480 \
        \nstop_\ncreate %self component:collider Player Point 0 0 \
        \nstop_ @onUpdate\n end_script>"), 0);
    manager.addComponent(std::string("component:spawner 3 8\
        1000 0 Source PosRad 0 45 10 0 \
        VelSpeed 65 AwayFromOrigin\
        1\
        component 0 <component:sprite NamelessSheet 0, 0>\
        component 0 <component:bounds destroy 0 0 320 480>"), 0);
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
