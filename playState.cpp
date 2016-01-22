#include "playState.h"
#include "SDL_image.h"
#include <iostream>

PlayState::PlayState(Window* w) : manager{}, window(w) {

}

void PlayState::handleInput() {

}

void PlayState::update(float rate) {
    manager.update(rate);
}

void PlayState::render(float remainder) {
    //renderSys.render(remainder);
}

PlayState::~PlayState() {
    window = nullptr;
}
