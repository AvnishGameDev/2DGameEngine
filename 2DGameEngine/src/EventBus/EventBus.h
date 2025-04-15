#pragma once

#include <typeindex>
#include <memory>
#include <list>
#include <map>

#include "Event.h"
#include "../Logger/Logger.h"

class IEventCallback
{
private:
    virtual void Call(Event& e) = 0;
    
public:
    virtual ~IEventCallback() = default;

    void Execute(Event& e)
    {
        Call(e);
    }
};

template <typename TOwner, typename TEvent>
class EventCallback : public IEventCallback
{
    typedef void (TOwner::*CallbackFunction)(TEvent&);
public:
    EventCallback(TOwner* ownerInstance, CallbackFunction callbackFunction)
    {
        this->ownerInstance = ownerInstance;
        this->callbackFunction = callbackFunction;
    }

    virtual ~EventCallback() override = default;
    
private:

    TOwner* ownerInstance;
    CallbackFunction callbackFunction;
    
    virtual void Call(Event& e) override
    {
        std::invoke(callbackFunction, ownerInstance, static_cast<TEvent&>(e));
    }
};

typedef std::list<std::unique_ptr<IEventCallback>> HandlerList;

class EventBus
{
public:
    EventBus()
    {
        Logger::Log("EventBus constructor");
    }

    ~EventBus()
    {
        Logger::Log("EventBus destructor");
    }

    // Clear the subscriber list
    void Reset()
    {
        subscribers.clear();
    }
    
    /**
     * @name Subscribe To event of type <T>
     * @brief In our implementation, a listener subscribes to an event \n
     * Example: eventBus->SubscribeToEvent<CollisionEvent>(this, &Game::OnCollision)
     */
    template <typename TEvent, typename TOwner>
    void SubscribeToEvent(TOwner* ownerInstance, void (TOwner::*callbackFunction)(TEvent& event))
    {
        if (!subscribers[typeid(TEvent)].get())
        {
            // Create Handler List for the type TEvent if it doesn't exist.
            subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
        }
        auto subscriber = std::make_unique<EventCallback<TOwner, TEvent>>(ownerInstance, callbackFunction);
        subscribers[typeid(TEvent)]->push_back(std::move(subscriber));
    }

    /**
     * @name Emit an event of type <T>
     * @brief In our implementation, as soon as something emits and event, we go ahead and execute all the listener callbacks \n
     * Example: eventBus->EmitEvent<CollisionEvent>(player, enemy);
     */
    template <typename TEvent, typename ...TArgs>
    void EmitEvent(TArgs&& ...args)
    {
        auto handlerList = subscribers[typeid(TEvent)].get();
        if (handlerList)
        {
            for (auto it = handlerList->begin(); it != handlerList->end(); it++)
            {
                auto handler = it->get();
                TEvent event(std::forward<TArgs>(args)...);
                handler->Execute(event);
            }
        }
    }

private:
    std::map<std::type_index, std::unique_ptr<HandlerList>> subscribers;
};
