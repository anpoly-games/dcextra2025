#pragma once

void register_player(eecs::Registry& reg, flecs::world& ecs);
eecs::EntityId create_player(eecs::Registry& reg, float x = 0.f, float z = 0.f, float rot = 0.f);

