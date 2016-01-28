#include "window.h"
#include <iostream>
using namespace std;

Window::Window() : window(nullptr), renderer(nullptr), width(640), height(480) {
    window = SDL_CreateWindow("Unnamed",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
}

Window::~Window() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
}

SDL_Renderer* const Window::getRenderer() const {
    return renderer;
}

void Window::setWidth(int32_t w) {
    width = w;
}

void Window::setHeight(int32_t h) {
    height = h;
}

int32_t Window::getWidth() {
    return width;
}

int32_t Window::getHeight() {
    return height;
}

Window::operator bool() const {
    return window && renderer;
}
