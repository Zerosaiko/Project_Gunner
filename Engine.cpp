#include "engine.h"
#include "window.h"
#include "SDL.h"
#include "playState.h"
#include "InputMap.h"
#include "component.h"
#include <iostream>
#include "SDL_gpu.h"

Engine::Engine() : window(), running(true),
    beginTime(SDL_GetPerformanceCounter()), currentTime(0.0f), currentFPS(60.0f) {

    registerAllComponents();
}

Engine::~Engine() {

    deregisterAllComponents();
    while(!stateStack.empty()) {
        popState();
    }
}

void Engine::run() {

    inputMap["P1Shot"] = SDL_SCANCODE_Z;
    inputMap["P1Bomb"] = SDL_SCANCODE_X;
    inputMap["P1Move_Up"] = SDL_SCANCODE_UP;
    inputMap["P1Move_Right"] = SDL_SCANCODE_RIGHT;
    inputMap["P1Move_Down"] = SDL_SCANCODE_DOWN;
    inputMap["P1Move_Left"] = SDL_SCANCODE_LEFT;
    inputMap["P1Focus"] = SDL_SCANCODE_LSHIFT;

    if (!window) {
        using namespace std;
        cout << "Creation of Window failed" << endl;
        running = false;
    }

    pushState(new PlayState(&window));

    beginTime = SDL_GetPerformanceCounter();
    currentTime = 1.0f / currentFPS;
    while(running) {
        GameState* currentState = peekState();
        for(SDL_Event e; SDL_PollEvent(&e);) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                if (currentState) {
                    window.setWidth(e.window.data1);
                    window.setHeight(e.window.data2);
                    GPU_SetWindowResolution(window.getWidth(), window.getHeight());
                }

            }
        }
        currentTime += (float)(SDL_GetPerformanceCounter() - beginTime) / SDL_GetPerformanceFrequency();
        while (currentTime < 0.5f / currentFPS) {
            SDL_Delay(1);
            currentTime += (float)(SDL_GetPerformanceCounter() - beginTime) / SDL_GetPerformanceFrequency();
        }
        beginTime = SDL_GetPerformanceCounter();
        if (currentState) {

            currentState->handleInput();
            // capped at 5 updates maximum before rendering
            if (currentTime > 4.0f / currentFPS) {currentTime = 4.0f / currentFPS;}
            for(; currentTime >= 1.0f / currentFPS; currentTime -= 1.0f / currentFPS) {
                currentState->update(1.0f / currentFPS);
            }

            GPU_Clear(window.getTarget());
            currentState->render(currentTime * currentFPS);
            GPU_Flip(window.getTarget());
        } else {
            currentTime = 0;
        }
    }
}

void Engine::pushState(GameState* state) {
    stateStack.push_back(state);
}

void Engine::popState() {
    if (!stateStack.empty()) {
        GameState* back = stateStack.back();
        stateStack.pop_back();
        delete back;

    }
}

GameState* Engine::peekState() {
    if (!stateStack.empty()) {
        return stateStack.back();
    }
    return nullptr;
}

bool Engine::isRunning() const {
    return running;
}

void Engine::setRunning(bool run) {
    running = run;
}
