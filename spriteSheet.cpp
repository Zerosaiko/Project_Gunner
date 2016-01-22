#include "spriteSheet.h"

SpriteSheet::SpriteSheet(SDL_Texture* srcTexture, int32_t cellWidth, int32_t cellHeight,
                          int32_t cellSepW, int32_t cellSepH) :
    texture(srcTexture), ownedTexture(false) {
    int32_t sheetWidth, sheetHeight;
    sheetWidth = 0;
    sheetHeight = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &sheetWidth, &sheetHeight);
    if (cellWidth > 0 && cellHeight > 0 && cellSepW >= 0 && cellSepH >= 0
        && sheetWidth >= cellWidth && sheetHeight >= cellHeight) {
        width = sheetWidth / (cellWidth + cellSepW);
        height = sheetHeight / (cellHeight + cellSepH);
        for(decltype(height) i = 0; i < height; i++) {
            for(decltype(width) j = 0; j < width; j++) {
                spriteBounds.push_back(SDL_Rect());
                spriteBounds.back().x = j * (cellHeight + cellSepH);
                spriteBounds.back().y = i * (cellWidth + cellSepW);
                spriteBounds.back().w = cellWidth;
                spriteBounds.back().h = cellHeight;
            }
        }
    }


}

SpriteSheet::~SpriteSheet() {
    if (texture && ownedTexture)
        SDL_DestroyTexture(texture);
    texture = nullptr;
}

SDL_Texture* const SpriteSheet::getTexture() const {
    return texture;
}

SDL_Rect* const SpriteSheet::getSprite(uint32_t cellPos) {
    if(*this)
        return &spriteBounds.at(cellPos);
    return nullptr;
}

SDL_Rect* const SpriteSheet::getSprite(uint32_t cellX, uint32_t cellY) {
    return getSprite(cellX + cellY*width);
}

SpriteSheet::operator bool() {
    return texture && !spriteBounds.empty();
}
