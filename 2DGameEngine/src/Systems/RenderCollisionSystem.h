#pragma once

#include <SDL.h>

#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"

class RenderCollisionSystem : public System
{
public:
    RenderCollisionSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<BoxColliderComponent>();
    }

    void Update(SDL_Renderer* renderer, SDL_Rect& camera)
    {
        for (auto entity : GetSystemEntities())
        {
            const auto transform = entity.GetComponent<TransformComponent>();
            const auto collider = entity.GetComponent<BoxColliderComponent>();

            SDL_Rect colliderRect = {
                static_cast<int>(transform.position.x + collider.offset.x - camera.x),
                static_cast<int>(transform.position.y + collider.offset.y - camera.y),
                static_cast<int>(collider.width * transform.scale.x),
                static_cast<int>(collider.height * transform.scale.y)
            };

            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &colliderRect);
        }
    }
};
