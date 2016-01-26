#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include <cstdint>
#include <vector>
#include "window.h"
#include "gameState.h"
#include "component.h"

// just a centralized place to initialize SDL and run the game loop
class Engine {

public:
    Engine();

    ~Engine();

    void run();

    void pushState(GameState* state);

    void popState();

    GameState* peekState();

    bool isRunning() const;

    void setRunning(bool run);

private:
    std::vector<GameState*> stateStack;
    Window* window;
    bool running;
    decltype(SDL_GetPerformanceCounter()) beginTime;
    float currentTime;
    float currentFPS;
};

#endif // ENGINE_H_INCLUDED
