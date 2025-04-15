#include "AssetStore.h"

#include "../Logger/Logger.h"

#include <SDL_image.h>

AssetStore::AssetStore()
{
    Logger::Log("AssetStore constructor");
}

AssetStore::~AssetStore()
{
    ClearAssets();
    Logger::Log("AssetStore destructor");
}

void AssetStore::ClearAssets()
{
    for (auto texture : textures)
    {
        SDL_DestroyTexture(texture.second);
    }
    textures.clear();

    for (auto font : fonts)
    {
        if (font.second)
            TTF_CloseFont(font.second);
    }
    fonts.clear();
}

void AssetStore::AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath)
{
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    textures.emplace(assetId, texture);

    Logger::Log("New texture added with id " + assetId);
}

void AssetStore::AddFont(const std::string& assetId, const std::string& filePath, int fontSize)
{
    fonts.emplace(assetId, TTF_OpenFont(filePath.c_str(), fontSize));
    Logger::Log("New font added with id " + assetId);
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId)
{
    return textures[assetId];
}

TTF_Font* AssetStore::GetFont(const std::string& assetId)
{
    return fonts[assetId];
}
