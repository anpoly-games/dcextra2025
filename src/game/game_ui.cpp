#include <eecs.h>
#include <raylib.h>

#include "../ui.h"

bool is_ui_blocks_input()
{
    return false;
}

void draw_ui(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    DrawFPS(20, height - 40);
}

