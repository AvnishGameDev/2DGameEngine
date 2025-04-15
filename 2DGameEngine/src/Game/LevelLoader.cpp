#include "LevelLoader.h"

#include <fstream>
#include <sstream>

#include "Game.h"
#include "../AssetStore/AssetStore.h"
#include "../ECS/ECS.h"

#include "../Components/RigidBodyComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/TextLabelComponent.h"

#include <SDL.h>

#include <sol/sol.hpp>

#include "../Components/ScriptComponent.h"

LevelLoader::LevelLoader()
{
    Logger::Log("LevelLoader constructor");
}

LevelLoader::~LevelLoader()
{
    Logger::Log("LevelLoader destructor");
}

void LevelLoader::LoadLevel(sol::state& lua, const std::unique_ptr<Registry>& registry, const std::unique_ptr<AssetStore>& assetStore, SDL_Renderer* renderer, int level)
{
    const std::string scriptPath = "assets/scripts/Level" + std::to_string(level) + ".lua";

    // Only loads the script and checks for script errors
    sol::load_result script = lua.load_file(scriptPath);

    if (!script.valid())
    {
        sol::error err = script;
        Logger::Err("Script at " + scriptPath + " has error " + err.what());
        return;
    }

    // Executes the lua script
    lua.script_file("assets/scripts/Level" + std::to_string(level) + ".lua");

    // Read the big table for the current level
    sol::table levelTable = lua["Level"];

    /****       Read the Level Assets       ****/
    sol::table assets = levelTable["assets"];

    int i = 0;
    while (true)
    {
        sol::optional<sol::table> hasAsset = assets[i]; // Check if asset exists at index 'i'
        if (hasAsset == sol::nullopt)
            break; // break if we've reached the end of assets table
        sol::table asset = assets[i];
        std::string assetType = asset["type"];
        std::string assetId = asset["id"];
        std::string assetFile = asset["file"];
        if (assetType == "texture")
        {
            assetStore->AddTexture(renderer, assetId, assetFile);
        }
        if (assetType == "font")
        {
            int fontSize = asset["font_size"];
            assetStore->AddFont(assetId, assetFile, fontSize);
        }
        i++;
    }

    /****       Read the Level Tilemap      ****/
    sol::table map = levelTable["tilemap"];
    std::string mapFilePath = map["map_file"];
    std::string mapTextureAssetId = map["texture_asset_id"];
    int mapNumRows = map["num_rows"];
    int mapNumCols = map["num_cols"];
    int tileSize = map["tile_size"];
    double mapScale = map["scale"];
    std::fstream mapFile;
    mapFile.open(mapFilePath);
    for (int y = 0; y < mapNumRows; y++) {
        for (int x = 0; x < mapNumCols; x++) {
            char ch;
            mapFile.get(ch);
            int srcRectY = std::atoi(&ch) * tileSize;
            mapFile.get(ch);
            int srcRectX = std::atoi(&ch) * tileSize;
            mapFile.ignore();

            Entity tile = registry->CreateEntity();
            tile.AddComponent<TransformComponent>(glm::vec2(x * (mapScale * tileSize), y * (mapScale * tileSize)), glm::vec2(mapScale, mapScale), 0.0);
            tile.AddComponent<SpriteComponent>(mapTextureAssetId, tileSize, tileSize, 0, false, srcRectX, srcRectY);
        }
    }
    mapFile.close();
    Game::mapWidth = mapNumCols * tileSize * mapScale;
    Game::mapHeight = mapNumRows * tileSize * mapScale;
    
    /****   Read the Level Entities and Components  ****/
    sol::table entities = levelTable["entities"];
    i = 0;
    while (true) {
        sol::optional<sol::table> hasEntity = entities[i];
        if (hasEntity == sol::nullopt) {
            break;
        }

        sol::table entity = entities[i];

        Entity newEntity = registry->CreateEntity();

        // Tag
        sol::optional<std::string> tag = entity["tag"];
        if (tag != sol::nullopt) {
            newEntity.Tag(entity["tag"]);
        }

        // Group
        sol::optional<std::string> group = entity["group"];
        if (group != sol::nullopt) {
            newEntity.Group(entity["group"]);
        }

        // Components
        sol::optional<sol::table> hasComponents = entity["components"];
        if (hasComponents != sol::nullopt) {
            // Transform
            sol::optional<sol::table> transform = entity["components"]["transform"];
            if (transform != sol::nullopt) {
                newEntity.AddComponent<TransformComponent>(
                    glm::vec2(
                        entity["components"]["transform"]["position"]["x"],
                        entity["components"]["transform"]["position"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["transform"]["scale"]["x"].get_or(1.0),
                        entity["components"]["transform"]["scale"]["y"].get_or(1.0)
                    ),
                    entity["components"]["transform"]["rotation"].get_or(0.0)
                );
            }

            // RigidBody
            sol::optional<sol::table> rigidbody = entity["components"]["rigidbody"];
            if (rigidbody != sol::nullopt) {
                newEntity.AddComponent<RigidBodyComponent>(
                    glm::vec2(
                        entity["components"]["rigidbody"]["velocity"]["x"].get_or(0.0),
                        entity["components"]["rigidbody"]["velocity"]["y"].get_or(0.0)
                    )
                );
            }

            // Sprite
            sol::optional<sol::table> sprite = entity["components"]["sprite"];
            if (sprite != sol::nullopt) {
                newEntity.AddComponent<SpriteComponent>(
                    entity["components"]["sprite"]["texture_asset_id"],
                    entity["components"]["sprite"]["width"],
                    entity["components"]["sprite"]["height"],
                    entity["components"]["sprite"]["z_index"].get_or(1),
                    entity["components"]["sprite"]["fixed"].get_or(false),
                    entity["components"]["sprite"]["src_rect_x"].get_or(0),
                    entity["components"]["sprite"]["src_rect_y"].get_or(0)
                );
            }

            // Animation
            sol::optional<sol::table> animation = entity["components"]["animation"];
            if (animation != sol::nullopt) {
                newEntity.AddComponent<AnimationComponent>(
                    entity["components"]["animation"]["num_frames"].get_or(1),
                    entity["components"]["animation"]["speed_rate"].get_or(1)
                );
            }

            // BoxCollider
            sol::optional<sol::table> collider = entity["components"]["boxcollider"];
            if (collider != sol::nullopt) {
                newEntity.AddComponent<BoxColliderComponent>(
                    entity["components"]["boxcollider"]["width"],
                    entity["components"]["boxcollider"]["height"],
                    glm::vec2(
                        entity["components"]["boxcollider"]["offset"]["x"].get_or(0),
                        entity["components"]["boxcollider"]["offset"]["y"].get_or(0)
                    )
                );
            }
            
            // Health
            sol::optional<sol::table> health = entity["components"]["health"];
            if (health != sol::nullopt) {
                newEntity.AddComponent<HealthComponent>(
                    static_cast<int>(entity["components"]["health"]["health_percentage"].get_or(100))
                );
            }
            
            // ProjectileEmitter
            sol::optional<sol::table> projectileEmitter = entity["components"]["projectile_emitter"];
            if (projectileEmitter != sol::nullopt) {
                newEntity.AddComponent<ProjectileEmitterComponent>(
                    glm::vec2(
                        entity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
                        entity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
                    ),
                    static_cast<int>(entity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1)) * 1000,
                    static_cast<int>(entity["components"]["projectile_emitter"]["projectile_duration"].get_or(10)) * 1000,
                    static_cast<int>(entity["components"]["projectile_emitter"]["hit_percentage_damage"].get_or(10)),
                    entity["components"]["projectile_emitter"]["friendly"].get_or(false)
                );
            }

            // CameraFollow
            sol::optional<sol::table> cameraFollow = entity["components"]["camera_follow"];
            if (cameraFollow != sol::nullopt) {
                newEntity.AddComponent<CameraFollowComponent>();
            }

            // KeyboardControlled
            sol::optional<sol::table> keyboardControlled = entity["components"]["keyboard_controller"];
            if (keyboardControlled != sol::nullopt) {
                newEntity.AddComponent<KeyboardControlledComponent>(
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["up_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["up_velocity"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["right_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["right_velocity"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["down_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["down_velocity"]["y"]
                    ),
                    glm::vec2(
                        entity["components"]["keyboard_controller"]["left_velocity"]["x"],
                        entity["components"]["keyboard_controller"]["left_velocity"]["y"]
                    )
                );
            }

            // Script Component
            sol::optional<sol::table> script = entity["components"]["on_update_script"];
            if (script != sol::nullopt)
            {
                sol::function func = entity["components"]["on_update_script"][0];
                newEntity.AddComponent<ScriptComponent>(func);
            }
        }
        i++;
    }
    
    // Adding assets to the asset store
    /*assetStore->AddTexture(renderer, "tank-image", "assets/images/tank-panther-right.png");
    assetStore->AddTexture(renderer, "truck-image", "assets/images/truck-ford-right.png");
    assetStore->AddTexture(renderer, "chopper-image", "assets/images/chopper-spritesheet.png");
    assetStore->AddTexture(renderer, "radar-image", "assets/images/radar.png");
    assetStore->AddTexture(renderer, "bullet-image", "assets/images/bullet.png");
    assetStore->AddTexture(renderer, "tree-image", "assets/images/tree.png");

    assetStore->AddFont("charriot-font", "assets/fonts/charriot.ttf", 20);
    assetStore->AddFont("pico8-font-5", "assets/fonts/pico8.ttf", 5);
    assetStore->AddFont("pico8-font-10", "assets/fonts/pico8.ttf", 10);

    // Load the tilemap
    assetStore->AddTexture(renderer, "tilemap-image", "assets/tilemaps/jungle.png");

#pragma region LectureCode
    /*int tileSize = 32;
    double tileScale = 2.0;
    int mapNumCols = 25;
    int mapNumRows = 20;

    std::fstream mapFile;
    mapFile.open("assets/tilemaps/jungle.map");

    for (int y=0; y < mapNumRows; y++)
    {
        for (int x=0; x < mapNumCols; x++)
        {
            char ch;
            mapFile.get(ch);
            int srcRectY = std::atoi(&ch) * tileSize;
            mapFile.get(ch);
            int srcRectX = std::atoi(&ch) * tileSize;
            mapFile.ignore();

            Entity tile = registry->CreateEntity();
            tile.AddComponent<TransformComponent>(glm::vec2(x*tileScale*tileSize, y*tileScale*tileSize), glm::vec2(tileScale, tileScale), 0.0);
            tile.AddComponent<SpriteComponent>("tilemap-image", tileSize, tileSize, srcRectX, srcRectY);
        }
    }
    mapFile.close();#1#
#pragma endregion /* LectureCode #1#
    
    // Map Properties
    const int tileSize = 32;
    const int tilemapCols = 10;
    const double tileScale = 2.0;
    
    std::string line;
    std::ifstream mapFile;
    mapFile.open("assets/tilemaps/jungle.map");

    int mapNumRows=0;
    int mapNumCols=0;
    for (mapNumRows = 0; std::getline(mapFile, line); mapNumRows++)
    {
        std::string value;
        std::stringstream ss(line);
        for (mapNumCols = 0; std::getline(ss, value, ','); mapNumCols++)
        {
            int val = std::stoi(value);

            int srcRectX = tileSize*(val%tilemapCols);
            int srcRectY = tileSize*(val/tilemapCols); // C++ always truncates down. (2.8=2, 2.4=2, 1.9=1, 0.8=0)
            
            Entity tile = registry->CreateEntity();
            tile.Group("tiles");
            tile.AddComponent<TransformComponent>(
                    glm::vec2(tileSize*mapNumCols*tileScale, tileSize*mapNumRows*tileScale),
                    glm::vec2(tileScale,tileScale),
                    0.0
                );
            tile.AddComponent<SpriteComponent>("tilemap-image", tileSize, tileSize, 0, false, srcRectX, srcRectY);
        }
    }
    mapFile.close();

    Game::mapWidth = mapNumCols * tileSize * tileScale;
    Game::mapHeight = mapNumRows * tileSize * tileScale;
    
    // Creating entities
    Entity chopper = registry->CreateEntity();
    chopper.Tag("player");
    chopper.AddComponent<TransformComponent>(glm::vec2(50, 50), glm::vec2(1, 1), 0.0);
    chopper.AddComponent<RigidBodyComponent>(glm::vec2(0, 0));
    chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 2);
    chopper.AddComponent<AnimationComponent>(2, 15, true);
    chopper.AddComponent<BoxColliderComponent>(32, 32);
    chopper.AddComponent<KeyboardControlledComponent>(glm::vec2(0, -80), glm::vec2(80, 0), glm::vec2(0, 80), glm::vec2(-80, 0));
    chopper.AddComponent<CameraFollowComponent>();
    chopper.AddComponent<HealthComponent>(100);
    chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(150, 150), 0, 4000, 10, true);
    
    Entity radar = registry->CreateEntity();
    radar.AddComponent<TransformComponent>(glm::vec2(Game::windowWidth - 74, 10), glm::vec2(1, 1), 0.0);
    radar.AddComponent<RigidBodyComponent>(glm::vec2(0, 0));
    radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 2, true);
    radar.AddComponent<AnimationComponent>(8, 5, true);
    
    Entity tank = registry->CreateEntity();
    tank.Group("enemies");
    tank.AddComponent<TransformComponent>(glm::vec2(500, 500), glm::vec2(1, 1), 0.0);
    tank.AddComponent<RigidBodyComponent>(glm::vec2(20, 0));
    tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 2);
    tank.AddComponent<BoxColliderComponent>(25, 18, glm::vec2(5, 7));
    //tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(100, 0), 5000, 3000, 10, false);
    tank.AddComponent<HealthComponent>(100);

    Entity truck = registry->CreateEntity();
    truck.Group("enemies");
    truck.AddComponent<TransformComponent>(glm::vec2(120, 500), glm::vec2(1, 1), 0.0);
    truck.AddComponent<RigidBodyComponent>(glm::vec2(0, 0));
    truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 1);
    truck.AddComponent<BoxColliderComponent>(25, 20, glm::vec2(5, 5));
    truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(0, 100), 2000, 5000, 10, false);
    truck.AddComponent<HealthComponent>(100);

    Entity treeA = registry->CreateEntity();
    treeA.Group("obstacles");
    treeA.AddComponent<TransformComponent>(glm::vec2(600, 495), glm::vec2(1, 1), 0.0);
    treeA.AddComponent<SpriteComponent>("tree-image", 16, 32, 2);
    treeA.AddComponent<BoxColliderComponent>(16, 32);

    Entity treeB = registry->CreateEntity();
    treeB.Group("obstacles");
    treeB.AddComponent<TransformComponent>(glm::vec2(400, 495), glm::vec2(1, 1), 0.0);
    treeB.AddComponent<SpriteComponent>("tree-image", 16, 32, 2);
    treeB.AddComponent<BoxColliderComponent>(16, 32);

    Entity label = registry->CreateEntity();
    SDL_Color green = {0, 255, 0, 255};
    label.AddComponent<TextLabelComponent>(glm::vec2(Game::windowWidth/2 - 40, 10), "CHOPPER 1.0", "charriot-font", green, true);*/
}
