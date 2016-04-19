#include "window.h"
#include <iostream>
using namespace std;

Window::Window() : screen(nullptr), width(640), height(480) {

    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    screen = GPU_Init(width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    /*window = SDL_CreateWindow("Unnamed",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);*/
}

Window::~Window() {
    /*
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    */
    GPU_Quit();
}

GPU_Target* const Window::getTarget() const {
    return screen;
}

void Window::setWidth(uint16_t w) {
    width = w;
}

void Window::setHeight(uint16_t h) {
    height = h;
}

uint16_t Window::getWidth() {
    return width;
}

uint16_t Window::getHeight() {
    return height;
}

Window::operator bool() const {
//    return window && renderer;
    return screen;
}
