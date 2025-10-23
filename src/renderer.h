#pragma once

#include "podval.h"

void register_renderer(eecs::Registry& reg, flecs::world& ecs);

struct LightStrength : public FVal {};

