#include "Game.h"

#include "LevelLoader.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>

#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/RenderCollisionSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/ProjectileLifecycleSystem.h"
#include "../Systems/RenderHealthBarSystem.h"
#include "../Systems/RenderTextSystem.h"
#include "../Systems/RenderDebugGuiSystem.h"
#include "../Systems/ScriptSystem.h"

int Game::windowHeight;
int Game::windowWidth;
int Game::mapHeight;
int Game::mapWidth;

Game::Game()
{
    isRunning = false; // Set to true after Initialization.
    isDebug = false;
    registry = std::make_unique<Registry>();
    assetStore = std::make_unique<AssetStore>();
    eventBus = std::make_unique<EventBus>();
    Logger::Log("Game constructor");
}

Game::~Game()
{
    Logger::Log("Game destructor");
}

void Game::Initialize()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        Logger::Err("Error initializing SDL.");
        return;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    windowWidth = 800; //displayMode.w;
    windowHeight = 600; //displayMode.h;
    
    window = SDL_CreateWindow(
                            nullptr,
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            windowWidth,
                            windowHeight,
                            SDL_WINDOW_BORDERLESS
                            );

    if (!window)
    {
        Logger::Err("Error creating SDL window.");
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        Logger::Err("Error creating SDL renderer.");
        return;
    }

    //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    // Initialize the camera view with the entire screen area
    camera.x = 0;
    camera.y = 0;
    camera.w = windowWidth;
    camera.h = windowHeight;

    // Initialize SDL_ttf
    if (TTF_Init())
    {
        Logger::Err("Error initializing SDL_ttf");
        return;
    }

    // Initialize ImGUI context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    
    isRunning = true;
}

void Game::Run()
{
    Setup(); 
    while (isRunning)
    {
        ProcessInput();
        Update();
        Render();
    }
}

void Game::Setup()
{
    // Add the systems that need to be processed in our game.
    registry->AddSystem<MovementSystem>();
    registry->AddSystem<RenderSystem>();
    registry->AddSystem<AnimationSystem>();
    registry->AddSystem<CollisionSystem>();
    registry->AddSystem<RenderCollisionSystem>();
    registry->AddSystem<DamageSystem>();
    registry->AddSystem<KeyboardControlSystem>();
    registry->AddSystem<CameraMovementSystem>();
    registry->AddSystem<ProjectileEmitSystem>();
    registry->AddSystem<ProjectileLifecycleSystem>();
    registry->AddSystem<RenderTextSystem>();
    registry->AddSystem<RenderHealthBarSystem>();
    registry->AddSystem<RenderDebugGuiSystem>();
    registry->AddSystem<ScriptSystem>();

    // Create the bindings between C++ and Lua
    registry->GetSystem<ScriptSystem>().CreateLuaBindings(lua);
    
    LevelLoader loader;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);
    loader.LoadLevel(lua, registry, assetStore, renderer, 2);
}

void Game::ProcessInput()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        // ImGui SDL input
        ImGui_ImplSDL2_ProcessEvent(&e);
        
        // Handle core SDL events (close window, key pressed, etc.)
        switch (e.type)
        {
        case SDL_KEYUP:
            break;
        case SDL_KEYDOWN:
            eventBus->EmitEvent<KeyPressedEvent>(e.key.keysym.sym);
            if (e.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
            }
            if (e.key.keysym.sym == SDLK_F1)
            {
                isDebug = !isDebug;
            }
            break;
        case SDL_QUIT:
            isRunning = false;
            break;
        default:
            break;
        }
    }
}

void Game::Update()
{
    /*int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPreviousFrame);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);*/

    double deltaTime = (SDL_GetTicks() - millisecsPreviousFrame) / 1000.0f;
    millisecsPreviousFrame = SDL_GetTicks();

    // Reset all event handlers
    eventBus->Reset();
    
    // Perform the subscription of the events for all systems
    registry->GetSystem<DamageSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<ProjectileEmitSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<MovementSystem>().SubscribeToEvents(eventBus);
    
    // Ask all the systems to update.
    registry->GetSystem<MovementSystem>().Update(deltaTime);
    registry->GetSystem<AnimationSystem>().Update();
    registry->GetSystem<CollisionSystem>().Update(eventBus);
    registry->GetSystem<DamageSystem>().Update();
    registry->GetSystem<CameraMovementSystem>().Update(camera);
    registry->GetSystem<ProjectileEmitSystem>().Update(registry);
    registry->GetSystem<ProjectileLifecycleSystem>().Update();
    registry->GetSystem<ScriptSystem>().Update(deltaTime, SDL_GetTicks());
    
    // Update the registry to process the entities that are waiting to be created/deleted
    registry->Update();
}

void Game::Render()
{
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255); // Background color.
    SDL_RenderClear(renderer);

    // Render Game Objects.
    registry->GetSystem<RenderSystem>().Update(renderer, assetStore, camera);
    registry->GetSystem<RenderTextSystem>().Update(renderer, assetStore, camera);
    registry->GetSystem<RenderHealthBarSystem>().Update(renderer, assetStore, camera);
    
    if (isDebug)
    {
        registry->GetSystem<RenderCollisionSystem>().Update(renderer, camera);
        registry->GetSystem<RenderDebugGuiSystem>().Update(renderer, registry, camera);
    }
    
    SDL_RenderPresent(renderer);
}

void Game::Destroy()
{
    assetStore->ClearAssets();

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
