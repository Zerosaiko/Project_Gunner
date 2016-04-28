#ifndef SPRITESHEET_H_INCLUDED
#define SPRITESHEET_H_INCLUDED

#include "SDL.h"
#include "window.h"
#include <vector>
#include <array>
#include <cstdint>
#include "SDL_gpu.h"

//  cannot atlas textures, but does allow usage of simple rectangle bound sprite sheets

class SpriteInfo {
private:
    GPU_Rect rect;
    std::array<float, 4> uvCoords;

public:
    SpriteInfo(float x, float y, float w, float h, float texW, float texH);

    GPU_Rect& getRect();

    std::array<float, 4>& getTexCoords();

};

class SpriteSheet {
public:

    SpriteSheet(GPU_Image* srcTexture, int32_t cellWidth, int32_t cellHeight,
                          int32_t cellSepW, int32_t cellSepH);

    ~SpriteSheet();

    SpriteInfo& getSprite(uint32_t cellPos);

    SpriteInfo& getSprite(uint32_t cellX, uint32_t cellY);

    GPU_Image* const getImage() const;

private:

    GPU_Image* texture;
    std::vector<SpriteInfo> spriteBounds;
    bool ownedTexture;
    uint32_t width;
    uint32_t height;
};


#endif // SPRITESHEET_H_INCLUDED
