#include "spriteSheet.h"
#include <iostream>

SpriteSheet::SpriteSheet(GPU_Image* srcTexture, int32_t cellWidth, int32_t cellHeight,
                          int32_t cellSepW, int32_t cellSepH) :
    texture(srcTexture), ownedTexture(false) {
    int32_t sheetWidth, sheetHeight;
    sheetWidth = texture->w;
    sheetHeight = texture->h;
    if (cellWidth > 0 && cellHeight > 0 && cellSepW >= 0 && cellSepH >= 0
        && sheetWidth >= cellWidth && sheetHeight >= cellHeight) {
        width = sheetWidth / (cellWidth + cellSepW);
        height = sheetHeight / (cellHeight + cellSepH);
        for(decltype(height) i = 0; i < height; i++) {
            for(decltype(width) j = 0; j < width; j++) {
                spriteBounds.push_back(
                        GPU_Rect{j * (cellHeight + cellSepH), i * (cellWidth + cellSepW), cellWidth, cellHeight}
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

GPU_Rect* SpriteSheet::getSprite(uint32_t cellPos) {
    if(*this)
        return &spriteBounds.at(cellPos);
    return nullptr;
}

GPU_Rect* SpriteSheet::getSprite(uint32_t cellX, uint32_t cellY) {
    return getSprite(cellX + cellY*width);
}

SpriteSheet::operator bool() const {
    return texture && !spriteBounds.empty();
}
