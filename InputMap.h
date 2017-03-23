#ifndef INPUTMAP_H_INCLUDED
#define INPUTMAP_H_INCLUDED

#include <unordered_map>
#include <vector>
#include <string>
#include <SDL.h>
#include <iostream>

extern std::unordered_map<std::string, SDL_Scancode> inputTypeMap;

class Input {

    enum class KeyState : uint8_t {
        None = 0x00,
        Pressed = 0x01,
        Held = 0x02,
        Released = 0x04

    };

    size_t keyCount;
    uint8_t const * SDLKeys;

    std::vector<KeyState> keys;

    friend std::ostream& operator<<(std::ostream &os, KeyState &ks);

public:
    Input();

    void update();

    bool keyReleased(SDL_Scancode key);

    bool keyPressed(SDL_Scancode key);

    bool keyHeld(SDL_Scancode key);

    bool keyDown(SDL_Scancode key);

    SDL_Scancode scancode(std::string &&keyName);

    SDL_Scancode scancodeSDL(std::string &&keyName);


};

extern Input input;



#endif // INPUTMAP_H_INCLUDED
