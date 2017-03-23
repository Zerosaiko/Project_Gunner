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

    while(!stateStack.empty()) {
        popState();
    }
    deregisterAllComponents();
}

void Engine::run() {

    inputTypeMap["P1ShotPri"] = SDL_SCANCODE_Z;
    inputTypeMap["P1Bomb"] = SDL_SCANCODE_X;
    inputTypeMap["P1ShotSec"] = SDL_SCANCODE_C;
    inputTypeMap["P1Move_Up"] = SDL_SCANCODE_UP;
    inputTypeMap["P1Move_Right"] = SDL_SCANCODE_RIGHT;
    inputTypeMap["P1Move_Down"] = SDL_SCANCODE_DOWN;
    inputTypeMap["P1Move_Left"] = SDL_SCANCODE_LEFT;
    inputTypeMap["P1Focus"] = SDL_SCANCODE_LSHIFT;

    if (!window) {
        using std::cout;
        using std::endl;
        cout << "Creation of Window failed" << endl;
        running = false;
    }

    pushState(new PlayState(&window));

    beginTime = SDL_GetPerformanceCounter();
    currentTime = 1.0f / currentFPS;
    while(running && !stateStack.empty()) {
        auto currentState = peekState();
        for(SDL_Event e; SDL_PollEvent(&e);) {
            if (e.type == SDL_QUIT) {
                running = false;
                return;
            } else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
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

            input.update();
            currentState->handleInput();
            // capped at 4 updates maximum before rendering
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
    stateStack.emplace_back(state);
    //state->update(1.0 / currentFPS);
}

void Engine::popState() {
    if (!stateStack.empty()) {
        stateStack.pop_back();
    }
}

std::shared_ptr<GameState> Engine::peekState() const {
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
