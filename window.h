#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "SDL.h"

#include <cstdint>

class Window {
public:
    Window();
    ~Window();

    SDL_Renderer* const getRenderer() const;

    void setWidth(int32_t w);

    void setHeight(int32_t h);

    int32_t getWidth();

    int32_t getHeight();

    explicit operator bool() const;

private:

    SDL_Window* window;
    SDL_Renderer* renderer;

    int32_t width;

    int32_t height;
};


#endif // WINDOW_H_INCLUDED
