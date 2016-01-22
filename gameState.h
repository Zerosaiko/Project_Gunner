#ifndef GAMESTATE_H_INCLUDED
#define GAMESTATE_H_INCLUDED

class GameState {
public:

    virtual void handleInput() = 0;

    virtual void update(float rate) = 0;

    virtual void render(float remainder) = 0;

    virtual ~GameState() = 0;


};

#endif // GAMESTATE_H_INCLUDED
