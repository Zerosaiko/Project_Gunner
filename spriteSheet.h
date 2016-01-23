#ifndef SPRITESHEET_H_INCLUDED
#define SPRITESHEET_H_INCLUDED

#include "SDL.h"
#include "window.h"
#include <vector>
#include <cstdint>

class SpriteSheet {
public:

    SpriteSheet(SDL_Texture* srcTexture, int32_t cellWidth, int32_t cellHeight,
                          int32_t cellSepW, int32_t cellSepH);

    SpriteSheet(SDL_Surface*& srcSurface, int32_t cellWidth, int32_t cellHeight,
                          int32_t cellSepW, int32_t cellSepH, Window* window);

    ~SpriteSheet();

    SDL_Rect const* getSprite(uint32_t cellPos) const;

    SDL_Rect const* getSprite(uint32_t cellX, uint32_t cellY) const;

    SDL_Texture* const getTexture() const;

    explicit operator bool() const;

private:

    SDL_Texture* texture;
    std::vector<SDL_Rect> spriteBounds;
    bool ownedTexture;
    uint32_t width;
    uint32_t height;
};


#endif // SPRITESHEET_H_INCLUDED
