#pragma once

#include "../ECS/ECS.h"

#include <SDL.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>

#include <glm/glm.hpp>

#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"

#define TO_DEG(x) x*(180/3.14)

class RenderDebugGuiSystem : public System
{
public:
    RenderDebugGuiSystem() = default;

    void Update(SDL_Renderer* renderer, const std::unique_ptr<Registry>& registry, const SDL_Rect& camera)
    {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        if (ImGui::Begin("Spawn enemies", nullptr))
        {
            static int pos[2];
            static int scale[2] = {1, 1};
            static float rotation;
            if (ImGui::CollapsingHeader("Transform Component"))
            {
                ImGui::InputInt2("Pos", pos);
                ImGui::SliderInt2("Scale", scale, 1, 10,  "%dx");
                ImGui::SliderAngle("Rotation", &rotation, 0, 360);
            }
            ImGui::Spacing();

            static int velocity[2] = {0, 0};
            if (ImGui::CollapsingHeader("RigidBody Component"))
            {
                ImGui::InputInt2("Velocity (px/sec)", velocity);
            }
            ImGui::Spacing();

            const char* sprites[] = {"tank-image", "truck-image"};
            static int selectedSpriteIndex = 0;
            static int spriteSize[2] = {32, 32};
            static int zIndex = 0;
            if (ImGui::CollapsingHeader("Sprite Component"))
            {
                ImGui::Combo("Sprite", &selectedSpriteIndex, sprites, IM_ARRAYSIZE(sprites));
                ImGui::InputInt2("Size", spriteSize);
                ImGui::SliderInt("zIndex", &zIndex, -100, 100);
            }
            ImGui::Spacing();

            static int colliderSize[2] = {32, 32};
            static int colliderOffset[2] = {0, 0};
            if (ImGui::CollapsingHeader("BoxColliderComponent"))
            {
                ImGui::InputInt2("Collider Size", colliderSize);
                ImGui::InputInt2("Collider Offset", colliderOffset);
            }
            ImGui::Spacing();

            static float projectileAngle = 0.0;
            static int projectileSpeed = 100;
            static int projectilesRepeat = 2;
            static int projectileDuration = 4;
            static int projectileDamage = 10;
            static bool projectileIsFriendly = false;
            if (ImGui::CollapsingHeader("Projectile Emitter Component"))
            {
                ImGui::SliderAngle("Angle", &projectileAngle, 0, 360);
                ImGui::SliderInt("Speed", &projectileSpeed, 10, 500);
                ImGui::InputInt("Projectiles Repeat (sec))", &projectilesRepeat);
                ImGui::InputInt("Projectile Duration", &projectileDuration);
                ImGui::InputInt("Projectile Damage", &projectileDamage);
                ImGui::Checkbox("Is Friendly", &projectileIsFriendly);
            }
            ImGui::Spacing();

            static int initialHealth = 100;
            if (ImGui::CollapsingHeader("Health Component"))
            {
                ImGui::SliderInt("Initial Health", &initialHealth, 1, 100, "%d%%");
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("Spawn"))
            {
                Entity enemy = registry->CreateEntity();
                enemy.Group("enemies");
                enemy.AddComponent<TransformComponent>(glm::vec2(pos[0], pos[1]), glm::vec2(scale[0], scale[1]), TO_DEG(rotation));
                enemy.AddComponent<RigidBodyComponent>(glm::vec2(velocity[0], velocity[1]));
                enemy.AddComponent<SpriteComponent>(sprites[selectedSpriteIndex], spriteSize[0], spriteSize[1], zIndex);
                enemy.AddComponent<BoxColliderComponent>(colliderSize[0], colliderSize[1], glm::vec2(colliderOffset[0], colliderOffset[1]));
                const double velX = glm::cos(projectileAngle) * projectileSpeed;
                const double velY = glm::sin(projectileAngle) * projectileSpeed;
                enemy.AddComponent<ProjectileEmitterComponent>(glm::vec2(velX, velY), projectilesRepeat*1000, projectileDuration*1000, projectileDamage, projectileIsFriendly);
                enemy.AddComponent<HealthComponent>(initialHealth);
            }
        }
        ImGui::End();

        // Display a small overlay window to display the map position using the mouse
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.8f);
        if (ImGui::Begin("Map coordinates", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove))
        {
            ImGui::Text(
                "Map coordinates (x=%d, y=%d)",
                static_cast<int>(ImGui::GetIO().MousePos.x + camera.x),
                static_cast<int>(ImGui::GetIO().MousePos.y + camera.y)
            );
        }
        ImGui::End();
        
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    }
};
