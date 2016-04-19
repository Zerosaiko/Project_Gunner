#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "SDL.h"
#include "SDL_gpu.h"

#include <cstdint>

class Window {
public:
    Window();
    ~Window();

    GPU_Target* const getTarget() const;

    void setWidth(uint16_t w);

    void setHeight(uint16_t h);

    uint16_t getWidth();

    uint16_t getHeight();

    explicit operator bool() const;

private:

    //SDL_Window* window;
    //SDL_Renderer* renderer;

    GPU_Target* screen;

    uint16_t width;

    uint16_t height;
};


#endif // WINDOW_H_INCLUDED
