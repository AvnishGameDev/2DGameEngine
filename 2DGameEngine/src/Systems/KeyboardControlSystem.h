#pragma once

#include "../Components/KeyboardControlledComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Events/KeyPressedEvent.h"
#include "../EventBus/EventBus.h"
#include "../ECS/ECS.h"

class KeyboardControlSystem : public System
{
public:
    KeyboardControlSystem()
    {
        RequireComponent<KeyboardControlledComponent>();
        RequireComponent<RigidBodyComponent>();
        RequireComponent<SpriteComponent>();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus)
    {
        eventBus->SubscribeToEvent(this, &KeyboardControlSystem::OnKeyPress);
    }

    void OnKeyPress(KeyPressedEvent& event)
    {
        for (auto entity : GetSystemEntities())
        {
            const auto keyboardControl = entity.GetComponent<KeyboardControlledComponent>();
            auto& sprite = entity.GetComponent<SpriteComponent>();
            auto& rigidBody = entity.GetComponent<RigidBodyComponent>();

            switch (event.key)
            {
            case SDLK_UP:
                rigidBody.velocity = keyboardControl.upVelocity;
                sprite.srcRect.y = sprite.height * 0;
                break;
            case SDLK_RIGHT:
                rigidBody.velocity = keyboardControl.rightVelocity;
                sprite.srcRect.y = sprite.height * 1;
                break;
            case SDLK_DOWN:
                rigidBody.velocity = keyboardControl.downVelocity;
                sprite.srcRect.y = sprite.height * 2;
                break;
            case SDLK_LEFT:
                rigidBody.velocity = keyboardControl.leftVelocity;
                sprite.srcRect.y = sprite.height * 3;
                break;
            default:
                break;
            }
        }
    }

    void Update()
    {
        
    }
};
