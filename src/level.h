#pragma once

#include <string>
#include <set>
#include <vector>
#include "editor/editor_ui.h"
#include "math.h"

void register_level(eecs::Registry& reg);

void load_level(eecs::Registry& reg, const char* filename);

// Only one step at a time, not a visibility check!
bool check_collision_dir(eecs::Registry& reg, const vec2i& pos, const vec2i& dir);

