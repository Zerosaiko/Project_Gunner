#include "spriteSheet.h"
#include <iostream>

SpriteSheet::SpriteSheet(GPU_Image* srcTexture, int32_t cellWidth, int32_t cellHeight,
                          int32_t cellSepW, int32_t cellSepH) :
    texture(srcTexture), ownedTexture(false) {
    int16_t sheetWidth, sheetHeight;
    sheetWidth = texture->w;
    sheetHeight = texture->h;

    if (cellWidth > 0 && cellHeight > 0 && cellSepW >= 0 && cellSepH >= 0
        && sheetWidth >= cellWidth && sheetHeight >= cellHeight) {
        width = sheetWidth / (cellWidth + cellSepW);
        height = sheetHeight / (cellHeight + cellSepH);
        for(decltype(height) i = 0; i < height; i++) {
            for(decltype(width) j = 0; j < width; j++) {
                spriteBounds.emplace_back(
                        j * (cellHeight + cellSepH), i * (cellWidth + cellSepW), cellWidth, cellHeight, sheetWidth, sheetHeight
                );
            }
        }
    }
}

SpriteSheet::~SpriteSheet() {
    GPU_FreeImage(texture);
    texture = nullptr;
}

GPU_Image* const SpriteSheet::getImage() const {
    return texture;
}

SpriteInfo& SpriteSheet::getSprite(uint32_t cellPos) {
        return spriteBounds.at(cellPos);
}

SpriteInfo& SpriteSheet::getSprite(uint32_t cellX, uint32_t cellY) {
    return getSprite(cellX + cellY*width);
}

SpriteInfo::SpriteInfo(float x, float y, float w, float h, float texW, float texH) : rect{x, y, w, h},
    uvCoords{x / texW, y / texH, (x + w) / texW, (y + h) / texH} {
}

GPU_Rect& SpriteInfo::getRect() {
    return rect;
}

std::array<float, 4>& SpriteInfo::getTexCoords() {
    return uvCoords;
}
