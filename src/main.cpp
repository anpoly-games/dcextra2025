#include <raylib.h>
#include <eecs.h>
#include <random>
#include <cstring>
#include <unordered_map>

#include "editor/editor_ui.h"
#include "ui.h"

#include "cam.h"
#include "game.h"
#include "level.h"
#include "tags.h"

static std::unordered_map<std::string, eecs::Registry*> registries;

int main(int argc, char** argv)
{
    int width = 1280;
    int height = 720;
    const char* levelToLoad = nullptr;//"level1";
    bool debugMode = false;
    for ( int i=1; i < argc; i++ )
    {
        if (strncmp(argv[i], "-l", 2)==0)
        {
            levelToLoad = argv[++i];
            printf("Loading level %s\n", levelToLoad);
        }
        
        if (strncmp(argv[i], "-d", 2)==0)
        {
          debugMode = true;
        }
    }

    ChangeDirectory(TextFormat("%s/../Resources/", GetApplicationDirectory()));

    InitWindow(width, height, "DC Extra Jam 2025");
    const int scrWidth = GetMonitorWidth(0);
    const int scrHeight = GetMonitorHeight(0);
    CloseWindow();

    int scaleFactor = std::max(1, std::min((scrWidth - 50) / width, (scrHeight - 50) / height));

    InitWindow(width * scaleFactor, height * scaleFactor, "DC Extra Jam 2025");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    SetExitKey(KEY_NULL);
    InitAudioDevice();

    eecs::Registry* reg = nullptr;

    if (debugMode)
    {
      eecs::EntityId eid = eecs::create_entity(*reg, "DebugMarker"); // existence of this entity means we are in debug mode
    }

    if (levelToLoad)
    {
        preload_levels(registries, width * scaleFactor, height * scaleFactor, scaleFactor); // maybe only in the game?
        auto itf = registries.find(levelToLoad);
        if (itf != registries.end())
            reg = itf->second;
        else
        {
            reg = new eecs::Registry{};
            create_ui_helper(*reg, width * scaleFactor, height * scaleFactor, scaleFactor);
            load_level(*reg, levelToLoad);
            registries[levelToLoad] = reg;
        }
        set_game_state(E_GAME);
    }
    else
    {
        reg = new eecs::Registry{};
        register_systems(*reg);
        init_new_world(*reg);
        preload_levels(registries, width * scaleFactor, height * scaleFactor, scaleFactor); // maybe only in the game?
        create_ui_helper(*reg, width * scaleFactor, height * scaleFactor, scaleFactor);
        registries["default"] = reg;
    }

    while (!WindowShouldClose())
    {
        pre_draw_call(*reg);
            begin_cam(*reg);
                eecs::step(*reg);
            end_cam(*reg);
            begin_postcam(*reg);
                render_cam(*reg);
                draw_ui(*reg, width * scaleFactor, height * scaleFactor, scaleFactor);
            end_postcam(*reg);
        reg = change_level(*reg, registries); // look for a change level request and execute it.
        bool shouldQuit = false;
        eecs::query_entities(*reg, [&](eecs::EntityId, Tag quitGame)
        {
            shouldQuit = true;
        }, COMPID(Tag, quitGame));
        if (shouldQuit)
        {
            CloseWindow();
            break;
        }
        assert(reg);
    }
    CloseAudioDevice();

    for ( auto r : registries )
    {
      delete r.second;
    }

    return 0;
}

eecs::Registry& get_registry(const std::string& name)
{
    auto itf = registries.find(name);
    assert(itf != registries.end());
    if (itf == registries.end())
    {
        static eecs::Registry temp;
        return temp;
    }
    return *itf->second;
}

