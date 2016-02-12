#include "playState.h"
#include "SDL_image.h"
#include "scriptcomponent.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 6000}, renderSys{&manager, 10000, window},
        scriptSys(&manager, 0), mInpSys(&manager, 1), boundsSys(&manager, 7500), colSys{&manager, 5999} {
    manager.addSystem(&moveSys);
    manager.addSystem(&renderSys);
    manager.addSystem(&scriptSys);
    manager.addSystem(&mInpSys);
    manager.addSystem(&boundsSys);
    manager.addSystem(&colSys);

    manager.createEntity(0);
    manager.tagManager.tagEntity("player", 0);
    manager.addComponent(std::string("component:script 2000 \
        <@onStart\
        \ncreate %self component:position 0, 0\
        \nstop_\ncreate %self\ncomponent:velocity 0, 0\
        \nstop_\ncreate %self\ncomponent:sprite NamelessSheet 0, 0\
        \nstop_\ncreate %self component:speed 80 \
        \nstop_\ncreate %self component:focusSpeed 35 \
        \nstop_\ncreate %self component:bounds block 0 0 320 480 \
        \nstop_\ncreate %self component:collider Player Point 0 0 \
        \nstop_ @onUpdate\n end_script>"), 0);
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
