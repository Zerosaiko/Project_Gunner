 #include "playState.h"
#include "SDL_image.h"
#include "scriptcomponent.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 99999}, renderSys{&manager, 100000, window},
        scriptSys(&manager, 0) {
    manager.addSystem(&moveSys);
    manager.addSystem(&renderSys);
    manager.addSystem(&scriptSys);

    manager.createEntity(0);
    manager.createEntity(1);
    manager.addChild(0, 1);
    manager.addComponent(std::string("component:script @start\ncreate %parent\ncomponent:displace 320, 0, -50, 75\nstop_\ncreate %parent\ncomponent:sprite NamelessSheet 0, 0\nstop_\ncreate %parent\ncomponent:displace 400, 500, -50, -90\nstop_stop_ end_script\n"), 1);

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
