#pragma once

#include <SDL.h>
#include <glm/glm.hpp>

#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../ECS/ECS.h"

class ProjectileEmitSystem : public System
{
public:
    ProjectileEmitSystem()
    {
        RequireComponent<ProjectileEmitterComponent>();
        RequireComponent<TransformComponent>();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus)
    {
        eventBus->SubscribeToEvent<KeyPressedEvent>(this, &ProjectileEmitSystem::OnKeyPress);
    }

    void OnKeyPress(KeyPressedEvent& event)
    {
        if (event.key == SDLK_SPACE)
        {
            for (auto entity : GetSystemEntities())
            {
                if (entity.HasTag("player"))
                {
                    const auto projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
                    const auto transform = entity.GetComponent<TransformComponent>();
                    const auto rigidBody = entity.GetComponent<RigidBodyComponent>();

                    glm::vec2 projectilePosition = transform.position;
                    if (entity.HasComponent<SpriteComponent>())
                    {
                        auto sprite = entity.GetComponent<SpriteComponent>();
                        projectilePosition.x += transform.scale.x * sprite.width / 2;
                        projectilePosition.y += transform.scale.y * sprite.height / 2;
                    }

                    glm::vec2 projectileVelocity = projectileEmitter.projectileVelocity;
                    int directionX = 0;
                    int directionY = 0;
                    if (rigidBody.velocity.x > 0) directionX = +1;
                    if (rigidBody.velocity.x < 0) directionX = -1;
                    if (rigidBody.velocity.y > 0) directionY = +1;
                    if (rigidBody.velocity.y < 0) directionY = -1;
                    projectileVelocity.x = projectileEmitter.projectileVelocity.x * directionX;
                    projectileVelocity.y = projectileEmitter.projectileVelocity.y * directionY;
                    
                    // Create a new projectile.
                    auto projectile = entity.registry->CreateEntity();
                    projectile.Group("projectiles");
                    projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1, 1), 0.0);
                    projectile.AddComponent<RigidBodyComponent>(projectileVelocity);
                    projectile.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 1);
                    projectile.AddComponent<BoxColliderComponent>(4, 4);
                    projectile.AddComponent<ProjectileComponent>(
                        projectileEmitter.isFriendly,
                        projectileEmitter.hitPercentDamage,
                        projectileEmitter.projectileDuration
                        );
                }
            }
        }
    }
    
    void Update(std::unique_ptr<Registry>& registry)
    {
        for (auto entity : GetSystemEntities())
        {
            auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
            const auto transform = entity.GetComponent<TransformComponent>();

            if (projectileEmitter.repeatFrequency == 0)
                continue;
            
            // Check if its time to re-emit a new projectile.
            if (SDL_GetTicks() - projectileEmitter.lastEmissionTime > projectileEmitter.repeatFrequency)
            {
                glm::vec2 projectilePosition = transform.position;
                if (entity.HasComponent<SpriteComponent>())
                {
                    auto sprite = entity.GetComponent<SpriteComponent>();
                    projectilePosition.x += transform.scale.x * sprite.width / 2;
                    projectilePosition.y += transform.scale.y * sprite.height / 2;
                }
                
                // Create a new projectile.
                auto projectile = registry->CreateEntity();
                projectile.Group("projectiles");
                projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1, 1), 0.0);
                projectile.AddComponent<RigidBodyComponent>(projectileEmitter.projectileVelocity);
                projectile.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 4);
                projectile.AddComponent<BoxColliderComponent>(4, 4);
                projectile.AddComponent<ProjectileComponent>(
                    projectileEmitter.isFriendly,
                    projectileEmitter.hitPercentDamage,
                    projectileEmitter.projectileDuration
                    );

                // Update the projectile emitter component's last emission time to the current milliseconds.
                projectileEmitter.lastEmissionTime = SDL_GetTicks();
            }
        }
    }
};
