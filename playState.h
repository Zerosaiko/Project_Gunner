#ifndef PLAYSTATE_H_INCLUDED
#define PLAYSTATE_H_INCLUDED

#include "gameState.h"
#include "EntityManager.h"
#include "EntitySystem.h"
#include "window.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "ScriptSystem.h"
#include "MovementInput.h"
#include "BoundsCorrection.h"
#include "CollisionSystem.h"
#include "SpawnSystem.h"
#include "DelaySystem.h"
#include "PlayerSystem.h"
#include "AnimationSystem.h"
#include "ShieldSystem.h"
#include "LifeTimerSystem.h"
#include "TransformSystems.h"
#include "Transform.h"

#include "sol.hpp"


class PlayState : public GameState{
public:

    PlayState(Window* w);

    virtual void handleInput();

    virtual void update(float rate);

    virtual void render(float remainder);

    virtual ~PlayState();

private:
    sol::state luaState;
    EntityManager manager;
    Window* window;

    TransformTree tfGraph;
    MovementSystem moveSys;
    RenderSystem renderSys;
    MovementInputSystem mInpSys;
    BoundsSystem boundsSys;
    CollisionSystem colSys;
    SpawnSystem spawnSys;
    TransformSyncSystem tfSyncSys;
    TransformCalcSystem tfCalcSys;
    DelaySystem delaySys;
    PauseSystem pauseSys;
    PlayerSystem playSys;
    AnimationSystem animSys;
    ShieldSystem shieldSys;
    LifeTimerSystem lifeTimerSys;

};

#endif // PLAYSTATE_H_INCLUDED
