#pragma once

#include "../ECS/ECS.h"

#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Events/CollisionEvent.h"

class MovementSystem : public System
{
public:
    MovementSystem()
    {
        RequireComponent<TransformComponent>();
        RequireComponent<RigidBodyComponent>();
    }

    void SubscribeToEvents(const std::unique_ptr<EventBus>& eventBus)
    {
        eventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::OnCollision);
    }

    void OnCollision(CollisionEvent& event)
    {
        Entity a = event.a;
        Entity b = event.b;

        if (a.BelongsToGroup("enemies") && b.BelongsToGroup("obstacles"))
        {
            OnEnemyCollideWithObstacle(a, b);
        }
        if (a.BelongsToGroup("obstacles") && b.BelongsToGroup("enemies"))
        {
            OnEnemyCollideWithObstacle(b, a);
        }
    }

    void OnEnemyCollideWithObstacle(Entity enemy, Entity obstacle)
    {
        if (enemy.HasComponent<RigidBodyComponent>() && enemy.HasComponent<SpriteComponent>())
        {
            auto& rigidBody = enemy.GetComponent<RigidBodyComponent>();
            auto& sprite = enemy.GetComponent<SpriteComponent>();

            if (rigidBody.velocity.x != 0)
            {
                rigidBody.velocity.x *= -1;
                sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            }
            if (rigidBody.velocity.y != 0)
            {
                rigidBody.velocity.y *= -1;
                sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
            }
        }
    }
    
    void Update(double deltaTime)
    {
        // Loop over all the entities that the system is interested in
        for (auto entity : GetSystemEntities())
        {
            // Update entity position based on its velocity every frame of the game loop.
            auto& transform = entity.GetComponent<TransformComponent>();
            const auto rigidBody = entity.GetComponent<RigidBodyComponent>();

            transform.position.x += rigidBody.velocity.x * static_cast<float>(deltaTime);
            transform.position.y += rigidBody.velocity.y * static_cast<float>(deltaTime);

            // Check if entity is outside the map boundaries(with a 100 px forgiving margin)
            const int margin = 100;
            
            bool isEntityOutsideMap = (
                transform.position.x < -margin ||
                transform.position.x > Game::mapWidth + margin ||
                transform.position.y < -margin ||
                transform.position.y > Game::mapHeight + margin
            );

            // Kill all entities that move outside the map boundaries
            if (isEntityOutsideMap && !entity.HasTag("player"))
            {
                entity.Kill();
            }

            if (entity.HasTag("player") && entity.HasComponent<SpriteComponent>())
            {
                const auto sprite = entity.GetComponent<SpriteComponent>();

                const int paddingLeft = 0;
                const int paddingRight = 0;
                const int paddingTop = 0;
                const int paddingBottom = 80;
                
                if (transform.position.x < 0 + paddingTop)
                    transform.position.x = 0 + paddingTop;
                if (transform.position.x > Game::mapWidth - sprite.width - paddingRight)
                    transform.position.x = Game::mapWidth - sprite.width - paddingRight;
                if (transform.position.y < 0 + paddingLeft)
                    transform.position.y = 0 + paddingLeft;
                if (transform.position.y > Game::mapHeight - sprite.height - paddingBottom)
                    transform.position.y = Game::mapHeight - sprite.height - paddingBottom;
            }
        }
    }
};
