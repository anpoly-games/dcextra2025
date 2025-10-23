#pragma once

#include "podval.h"
#include <string>

// some settings for the game in the ecs format as it's easier to access it this way
struct OptInstantStep : public BVal {};
struct OptMusicVolume : public FVal {};
struct OptSfxVolume : public FVal {};

void register_settings(flecs::world& ecs);
void create_settings(flecs::world& ecs);
std::string save_settings(flecs::world& ecs);
void load_settings(flecs::world& ecs, const std::string& json);

