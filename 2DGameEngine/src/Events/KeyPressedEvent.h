#pragma once

#include <SDL.h>

#include "../EventBus/Event.h"
#include "../ECS/ECS.h"

class KeyPressedEvent : public Event
{
public:
    SDL_Keycode key;
    
    KeyPressedEvent(SDL_Keycode key) : key(key) {}
};
