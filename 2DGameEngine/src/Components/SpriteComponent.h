#pragma once

#include <string>

#include <SDL.h>

struct SpriteComponent
{
    std::string assetId;
    int width;
    int height;
    int zIndex;
    bool isFixed; // For UI
    SDL_Rect srcRect;
    SDL_RendererFlip flip;
    
    SpriteComponent(const std::string& assetId = "", int width=1, int height=1, int zIndex = 0, bool isFixed = false, int srcRectX = 0, int srcRectY = 0, const SDL_RendererFlip& flip = SDL_FLIP_NONE)
    {
        this->assetId = assetId;
        this->width = width;
        this->height = height;
        this->zIndex = zIndex;
        this->isFixed = isFixed;
        this->srcRect = {srcRectX, srcRectY, width, height};
        this->flip = flip;
    }
};
