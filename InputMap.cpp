#include "InputMap.h"
#include <iostream>

std::unordered_map<std::string, SDL_Scancode> inputTypeMap;

Input input;

Input::Input() : keyCount(0){
    int32_t keyCount;
    SDLKeys = SDL_GetKeyboardState(&keyCount);
    this->keyCount = keyCount;
    keys.resize(this->keyCount);

}

void Input::update() {
    for (size_t key = 0; key < keyCount; key++) {
        if (keys[key] == KeyState::Released) {
            keys[key] = KeyState::None;
        } else if (SDLKeys[key]) {
            if (keys[key] == KeyState::None) {
                keys[key] = KeyState::Pressed;
            }
            else if (keys[key] == KeyState::Pressed){
                keys[key] = KeyState::Held;
            }
        } else {
            if (keys[key] != KeyState::None){
                keys[key] = KeyState::Released;
            }
        }

    }

}

bool Input::keyReleased(SDL_Scancode key) {
    return keys[key] == KeyState::Released;
}

bool Input::keyPressed(SDL_Scancode key) {
    return keys[key] == KeyState::Pressed;
}

bool Input::keyHeld(SDL_Scancode key) {
    return keys[key] == KeyState::Held;

}

bool Input::keyDown(SDL_Scancode key) {

    return keys[key] == KeyState::Held || keys[key] == KeyState::Pressed;
}

SDL_Scancode Input::scancode(std::string &&keyName) {
    return inputTypeMap.at(keyName);
}

SDL_Scancode Input::scancodeSDL(std::string &&keyName) {
    return SDL_GetScancodeFromName(keyName.c_str());
}

std::ostream& operator<<(std::ostream &os, Input::KeyState &ks) {
    return os << static_cast<uint16_t>(ks);
}
