#include "window.h"
#include <iostream>
using namespace std;

Window::Window() : window(nullptr), renderer(nullptr) {
    window = SDL_CreateWindow("Unnamed",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if (window)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

Window::~Window() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
}

SDL_Renderer* const Window::getRenderer() const {
    return renderer;
}

Window::operator bool() const {
    return window && renderer;
}
