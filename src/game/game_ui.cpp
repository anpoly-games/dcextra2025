#include <eecs.h>
#include <raylib.h>

#include "../ui.h"
#include "interactables.h"

bool is_ui_blocks_input()
{
    return false;
}

void draw_ui(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    draw_interactables(reg, width, height, scaleFactor);
    DrawFPS(20, height - 40);
}

