#pragma once

#include <memory>

#include "SDL_render.h"
#include <sol/sol.hpp>

class AssetStore;
class Registry;

class LevelLoader
{
public:
    LevelLoader();
    ~LevelLoader();

    void LoadLevel(sol::state& lua, const std::unique_ptr<Registry>& registry, const std::unique_ptr<AssetStore>& assetStore, SDL_Renderer* renderer, int level);
};
