#include "playState.h"
#include "SDL_image.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w), moveSys{&manager, 99999}, renderSys{&manager, 100000, window} {
    manager.addSystem(&moveSys);
    manager.addSystem(&renderSys);

    manager.createEntity(0);
    manager.addComponent(std::string("component:displace 0, 0, 100, 100"), 0);
    manager.addComponent(std::string("component:sprite NamelessSheet 0, 0"), 0);

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
