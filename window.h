#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "SDL.h"
class Window {
public:
    Window();
    ~Window();

    SDL_Renderer* const getRenderer() const;

    explicit operator bool() const;

private:

    SDL_Window* window;
    SDL_Renderer* renderer;
};


#endif // WINDOW_H_INCLUDED
