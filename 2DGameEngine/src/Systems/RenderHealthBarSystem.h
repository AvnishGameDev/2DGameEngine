#pragma once

#include "../Components/TransformComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/SpriteComponent.h"
#include "../AssetStore/AssetStore.h"
#include "../ECS/ECS.h"

#include <SDL.h>

class RenderHealthBarSystem : public System
{
public:
    RenderHealthBarSystem()
    {
        RequireComponent<HealthComponent>();
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
    }

    void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const SDL_Rect& camera)
    {
        for (auto entity : GetSystemEntities())
        {
            const auto transform = entity.GetComponent<TransformComponent>();
            const auto sprite = entity.GetComponent<SpriteComponent>();
            const auto health = entity.GetComponent<HealthComponent>();

            // Draw the health bar with the correct color for the percentage
            SDL_Color healthBarColor = {255, 255, 255, 255};

            if (health.healthPercentage >= 0 && health.healthPercentage < 40)
                healthBarColor = {255, 0, 0, 255};
            if (health.healthPercentage >= 40 && health.healthPercentage < 80)
                healthBarColor = {255, 255, 0, 255};
            if (health.healthPercentage >= 80 && health.healthPercentage <= 100)
                healthBarColor = {0, 255, 0, 255};

            // Position the health bar indicator in the middle-bottom part of the entity sprite
            int healthBarWidth = 15;
            int healthBarHeight = 3;
            double healthBarPosX = (transform.position.x + (sprite.width * transform.scale.x)) - camera.x;
            double healthBarPosY = (transform.position.y) - camera.y;

            SDL_Rect healthBarRectangle = {
                static_cast<int>(healthBarPosX),
                static_cast<int>(healthBarPosY),
                static_cast<int>(healthBarWidth * (health.healthPercentage / 100.0)),
                static_cast<int>(healthBarHeight)
            };
            SDL_SetRenderDrawColor(renderer, healthBarColor.r, healthBarColor.g, healthBarColor.b, 255);
            SDL_RenderFillRect(renderer, &healthBarRectangle);

            // Render the health percentage text label indicator
            std::string healthText = std::to_string(health.healthPercentage);
            
            SDL_Surface* surface = TTF_RenderText_Blended(assetStore->GetFont("pico8-font-5"), healthText.c_str(), healthBarColor);
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            
            int labelWidth = 0;
            int labelHeight = 0;
            SDL_QueryTexture(texture, nullptr, nullptr, &labelWidth, &labelHeight);
            SDL_Rect healthBarTextRectangle = {
                static_cast<int>(healthBarPosX),
                static_cast<int>(healthBarPosY) + 5,
                labelWidth,
                labelHeight
            };

            SDL_RenderCopy(renderer, texture, nullptr, &healthBarTextRectangle);
            SDL_DestroyTexture(texture);
        }
    }
};
