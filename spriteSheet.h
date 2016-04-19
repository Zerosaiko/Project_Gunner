#ifndef SPRITESHEET_H_INCLUDED
#define SPRITESHEET_H_INCLUDED

#include "SDL.h"
#include "window.h"
#include <vector>
#include <cstdint>
#include "SDL_gpu.h"

//  cannot atlas textures, but does allow usage of simple rectangle bound sprite sheets
class SpriteSheet {
public:

    SpriteSheet(GPU_Image* srcTexture, int32_t cellWidth, int32_t cellHeight,
                          int32_t cellSepW, int32_t cellSepH);

    ~SpriteSheet();

    GPU_Rect* getSprite(uint32_t cellPos);

    GPU_Rect* getSprite(uint32_t cellX, uint32_t cellY);

    GPU_Image* const getImage() const;

    explicit operator bool() const;

private:

    GPU_Image* texture;
    std::vector<GPU_Rect> spriteBounds;
    bool ownedTexture;
    uint32_t width;
    uint32_t height;
};


#endif // SPRITESHEET_H_INCLUDED
