#pragma once
#include <memory>

#include "SDL_rect.h"
#include <sol/sol.hpp>

//const int FPS = 60;
//const int MILLISECS_PER_FRAME = 1000 / FPS; // 1S = 1000ms

struct SDL_Window;
struct SDL_Renderer;
class Registry;
class AssetStore;
class EventBus;

class Game
{
public:
    Game();
    ~Game();

    void Initialize();
    void Run();
    void Setup();
    void ProcessInput();
    void Update();
    void Render();
    void Destroy();

    static int windowWidth;
    static int windowHeight;
    static int mapWidth;
    static int mapHeight;

private:
    bool isRunning;
    bool isDebug;
    
    int millisecsPreviousFrame = 0;
    
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Rect camera;

    sol::state lua;
    
    std::unique_ptr<Registry> registry;
    std::unique_ptr<AssetStore> assetStore;
    std::unique_ptr<EventBus> eventBus;
};
