#pragma once

#include <string>
#include <map>

#include <SDL.h>
#include <SDL_ttf.h>

class AssetStore
{
public:

    AssetStore();
    ~AssetStore();

    void ClearAssets();
    void AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath);
    void AddFont(const std::string& assetId, const std::string& filePath, int fontSize);
    SDL_Texture* GetTexture(const std::string& assetId);
    TTF_Font* GetFont(const std::string& assetId);
    
private:
    std::map<std::string, SDL_Texture*> textures;
    std::map<std::string, TTF_Font*> fonts;
    // TODO: create a map for audio
};
