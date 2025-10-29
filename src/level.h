#pragma once

#include <string>
#include <set>
#include <vector>
#include <unordered_map>

#include "editor/editor_ui.h"
#include "math.h"

void register_level(eecs::Registry& reg);

void load_level(eecs::Registry& reg, const char* filename);
eecs::Registry* change_level(eecs::Registry& reg, std::unordered_map<std::string, eecs::Registry*>& registries);

// Only one step at a time, not a visibility check!
bool check_collision_dir(eecs::Registry& reg, const vec2i& pos, const vec2i& dir);

