#include <raylib.h>
#include <eecs.h>
#include <random>
#include <cstring>

#include "editor/editor_ui.h"
#include "ui.h"

#include "cam.h"
#include "game.h"
#include "level.h"
#include "tags.h"

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

    eecs::Registry reg;

    if (debugMode)
    {
      eecs::EntityId eid = eecs::create_entity(reg, "DebugMarker"); // existence of this entity means we are in debug mode
    }

    if (levelToLoad)
    {
        load_level(reg, levelToLoad);
    }
    else
    {
        register_systems(reg);
        init_new_world(reg);
        create_ui_helper(reg, width * scaleFactor, height * scaleFactor, scaleFactor);
    }

    while (!WindowShouldClose())
    {
        pre_draw_call(reg);
        BeginDrawing();
            ClearBackground(BLACK);
            update_cam(reg);
                eecs::step(reg);

            EndMode3D();
            // TODO: editor switch
            draw_ui(reg, width * scaleFactor, height * scaleFactor, scaleFactor);
        EndDrawing();
        change_level(reg); // look for a change level request and execute it.
    }
    CloseAudioDevice();
    return 0;
}

