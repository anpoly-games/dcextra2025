#pragma once

#include "podval.h"

void register_cam(eecs::Registry& reg, flecs::world& ecs);
eecs::EntityId create_cam(eecs::Registry& reg);

void update_cam(eecs::Registry& reg);

Vector3 GetWorldToScreen3dEx(Vector3 position, Camera camera);

