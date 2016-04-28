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

class PlayState : public GameState{
public:

    PlayState(Window* w);

    virtual void handleInput();

    virtual void update(float rate);

    virtual void render(float remainder);

    virtual ~PlayState();

private:
    EntityManager manager;
    Window* window;
    MovementSystem moveSys;
    RenderSystem renderSys;
    MovementInputSystem mInpSys;
    BoundsSystem boundsSys;
    CollisionSystem colSys;
    SpawnSystem spawnSys;
    PositionSyncSystem posSyncSys;
    DelaySystem delaySys;
    PauseSystem pauseSys;
    PlayerSystem playSys;
    AnimationSystem animSys;
    ShieldSystem shieldSys;
    LifeTimerSystem lifeTimerSys;

};

#endif // PLAYSTATE_H_INCLUDED
