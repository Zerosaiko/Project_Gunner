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
    manager.tagManager.tagEntity("testTag", 0);
    manager.groupManager.groupEntity("groupTag", 0);
    manager.addComponent(std::string("component:script 2000 @onStart\ncreate %parent component:position 0, 0\nstop_\ncreate %parent\ncomponent:velocity 60, 60\nstop_\ncreate %parent\ncomponent:sprite NamelessSheet 0, 0\nstop_\n @onUpdate\n remove %parent velocity\nend_script"), 1);
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
