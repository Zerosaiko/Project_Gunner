#include "playState.h"
#include "SDL_image.h"
#include "scriptcomponent.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 5000}, renderSys{&manager, 10000, window},
        scriptSys(&manager, 0), mInpSys(&manager, 1), boundsSys(&manager, 7500) {
    manager.addSystem(&moveSys);
    manager.addSystem(&renderSys);
    manager.addSystem(&scriptSys);
    manager.addSystem(&mInpSys);
    manager.addSystem(&boundsSys);

    manager.createEntity(0);
    manager.createEntity(1);
    manager.addChild(0, 1);
    manager.tagManager.tagEntity("player", 0);
    manager.groupManager.groupEntity("groupTag", 0);
    manager.addComponent(std::string("component:script 2000 \
        @onStart\
        \ncreate %parent component:position 0, 0\
        \nstop_\ncreate %parent\ncomponent:velocity 0, 0\
        \nstop_\ncreate %parent\ncomponent:sprite NamelessSheet 0, 0\
        \nstop_\ncreate %parent component:speed 80 \
        \nstop_\ncreate %parent component:focusSpeed 35 \
        \nstop_\ncreate %parent component:bounds block 0 0 320 480 \
        \n@onUpdate\n end_script"), 1);
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
