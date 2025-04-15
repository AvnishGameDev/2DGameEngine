#pragma once

#include "../Components/ScriptComponent.h"
#include "../Components/TransformComponent.h"

#include "../ECS/ECS.h"

#include <tuple> // Expected by Sol to return 2 values

std::tuple<double, double> GetEntityPosition(Entity entity)
{
    if (entity.HasComponent<TransformComponent>())
    {
        const auto transform = entity.GetComponent<TransformComponent>();
        return std::make_tuple(transform.position.x, transform.position.y);
    }
    else
    {
        Logger::Err("Trying to get position of entity with no TransformComponent!");
        return std::make_tuple(0.0, 0.0);
    }
}

std::tuple<double, double> GetEntityVelocity(Entity entity)
{
    if (entity.HasComponent<RigidBodyComponent>())
    {
        const auto rigidBody = entity.GetComponent<RigidBodyComponent>();
        return std::make_tuple(rigidBody.velocity.x, rigidBody.velocity.y);
    }
    else
    {
        Logger::Err("Trying to get velocity of entity with no RigidBody Component!");
        return std::make_tuple(0.0, 0.0);
    }
}

void SetEntityPosition(Entity entity, double x, double y)
{
    if (entity.HasComponent<TransformComponent>())
    {
        auto& transform = entity.GetComponent<TransformComponent>();
        transform.position.x = x;
        transform.position.y = y;
    }
    else
    {
        Logger::Err("Trying to set position of an entity with no TransformComponent!");
    }
}

void SetEntityVelocity(Entity entity, double x, double y)
{
    if (entity.HasComponent<RigidBodyComponent>())
    {
        auto& rigidBody = entity.GetComponent<RigidBodyComponent>();
        rigidBody.velocity.x = x;
        rigidBody.velocity.y = y;
    }
    else
    {
        Logger::Err("Trying to set velocity of an entity with no RigidBody Component!");
    }
}

void SetEntityRotation(Entity entity, double rotation)
{
    if (entity.HasComponent<TransformComponent>())
    {
        auto& transform = entity.GetComponent<TransformComponent>();
        transform.rotation = rotation;
    }
    else
    {
        Logger::Err("Trying to set rotation of an entity with no TransformComponent!");
    }
}

void SetEntityAnimationFrame(Entity entity, int frame)
{
    if (entity.HasComponent<AnimationComponent>())
    {
        auto& animation = entity.GetComponent<AnimationComponent>();
        animation.currentFrame = frame;
    }
    else
    {
        Logger::Err("Trying to set the animation frame of an entity with no AnimationComponent!");
    }
}

void SetProjectileVelocity(Entity entity, double x, double y)
{
    if (entity.HasComponent<ProjectileEmitterComponent>())
    {
        auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
        projectileEmitter.projectileVelocity.x = x;
        projectileEmitter.projectileVelocity.y = y;
    }
    else
    {
        Logger::Err("Trying to set ProjectileVelocity of an entity with no ProjectileEmitterComponent!");
    }
}

class ScriptSystem : public System
{
public:
    ScriptSystem()
    {
        RequireComponent<ScriptComponent>();
    }

    void CreateLuaBindings(sol::state& lua)
    {
        // Create the "entity" usertype for Lua
        lua.new_usertype<Entity>(
            "entity",
            "get_id", &Entity::GetId,
            "destroy", &Entity::Kill,
            "has_tag", &Entity::HasTag,
            "belongs_to_group", &Entity::BelongsToGroup
            );
        
        // Create all the bindings between C++ and Lua functions
        lua.set_function("get_position", GetEntityPosition);
        lua.set_function("get_velocity", GetEntityVelocity);
        lua.set_function("set_position", SetEntityPosition);
        lua.set_function("set_velocity", SetEntityVelocity);
        lua.set_function("set_rotation", SetEntityRotation);
        lua.set_function("set_animation_frame", SetEntityAnimationFrame);
        lua.set_function("set_projectile_velocity", SetProjectileVelocity);
    }

    void Update(double deltaTime, int ellapsedTime)
    {
        // Loop all the entities that have a script component and invoke their Lua function
        for (auto entity : GetSystemEntities())
        {
            auto& script = entity.GetComponent<ScriptComponent>();
            script.func(entity, deltaTime, ellapsedTime); // here is where we invoke a sol::function
        }
    }
};
