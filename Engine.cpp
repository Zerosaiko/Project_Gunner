#include "engine.h"
#include "window.h"
#include "SDL.h"
#include "SDL_image.h"
#include "playState.h"
#include "renderable.h"
#include "displace.h"
#include "scriptcomponent.h"
#include <iostream>

Engine::Engine() : stateStack{}, running(true),
    beginTime(SDL_GetPerformanceCounter()), currentTime(0.0f), currentFPS(60.0f) {

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    window = new Window();
}

Engine::~Engine() {

    deregisterAllComponents();
    while(!stateStack.empty()) {
        popState();
    }
    delete window;
    SDL_Quit();
    IMG_Quit();
}

void Engine::run() {

    Component<Renderable::name, Renderable>::registerComponent();
    Component<Script::name, Script>::registerComponent();
    Displace::registerComponent();

    if (!window) {
        using namespace std;
        cout << "Creation of Window failed" << endl;
        running = false;
    }

    SDL_SetRenderDrawBlendMode(window->getRenderer(), SDL_BLENDMODE_BLEND);

    pushState(new PlayState(window));

    beginTime = SDL_GetPerformanceCounter();
    currentTime = 1.0f / currentFPS;
    while(running) {
        for(SDL_Event e; SDL_PollEvent(&e);) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }
        GameState* currentState = peekState();
        if (currentTime < 0.1f / currentFPS)
            SDL_Delay(1);
        currentTime += (float)(SDL_GetPerformanceCounter() - beginTime) / SDL_GetPerformanceFrequency();
        beginTime = SDL_GetPerformanceCounter();
        if (currentState) {

            currentState->handleInput();
            // capped at 3 updates before rendering
            if (currentTime > 3.0f / 60.0f) currentTime = 3.0f / 60.0f;
            for(; currentTime >= 1.0f / currentFPS; currentTime -= 1.0f / currentFPS) {
                currentState->update(1.0f / currentFPS);
            }
            SDL_RenderClear(window->getRenderer());
            currentState->render(currentTime);
            SDL_RenderPresent(window->getRenderer());
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
