#ifndef INPUTMAP_H_INCLUDED
#define INPUTMAP_H_INCLUDED

#include <unordered_map>
#include <string>
#include <SDL.h>

extern std::unordered_map<std::string, SDL_Scancode> inputMap;

#endif // INPUTMAP_H_INCLUDED
