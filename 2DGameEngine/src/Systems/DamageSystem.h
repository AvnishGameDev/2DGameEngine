#pragma once

#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/HealthComponent.h"
#include "../Events/CollisionEvent.h"
#include "../EventBus/EventBus.h"
#include "../ECS/ECS.h"

class DamageSystem : public System
{
public:
    DamageSystem()
    {
        RequireComponent<BoxColliderComponent>();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus)
    {
        eventBus->SubscribeToEvent<CollisionEvent>(this, &DamageSystem::OnCollision);
    }

    void OnCollision(CollisionEvent& event)
    {
        Entity a = event.a;
        Entity b = event.b;

        if (a.BelongsToGroup("projectiles") && b.HasTag("player"))
        {
            OnProjectileHitPlayer(a, b);
        }

        if (b.BelongsToGroup("projectiles") && a.HasTag("player"))
        {
            OnProjectileHitPlayer(b, a);
        }

        if (a.BelongsToGroup("projectiles") && b.BelongsToGroup("enemies"))
        {
            OnProjectileHitEnemy(a, b);
        }

        if (b.BelongsToGroup("projectiles") && a.BelongsToGroup("enemies"))
        {
            OnProjectileHitEnemy(b, a);
        }
    }

    void OnProjectileHitPlayer(Entity projectile, Entity player)
    {
        auto projectileComponent = projectile.GetComponent<ProjectileComponent>();

        if (!projectileComponent.isFriendly)
        {
            auto& health = player.GetComponent<HealthComponent>();

            // Subtract the health of the player
            health.healthPercentage -= projectileComponent.hitPercentDamage;

            // Kill the player when health reaches zero
            if (health.healthPercentage <= 0)
            {
                player.Kill();
            }

            // Kill the projectile
            projectile.Kill();
        }
    }

    void OnProjectileHitEnemy(Entity projectile, Entity enemy)
    {
        auto projectileComponent = projectile.GetComponent<ProjectileComponent>();

        if (projectileComponent.isFriendly)
        {
            auto& health = enemy.GetComponent<HealthComponent>();

            // Subtract the health of the player
            health.healthPercentage -= projectileComponent.hitPercentDamage;

            // Kill the player when health reaches zero
            if (health.healthPercentage <= 0)
            {
                enemy.Kill();
            }

            // Kill the projectile
            projectile.Kill();
        }
    }
    
    void Update()
    {
        
    }
};
