#pragma once

void register_systems(eecs::Registry& reg);
eecs::EntityId init_new_world(eecs::Registry& reg);
void restart_world(eecs::Registry& reg);

void init_app(eecs::Registry& reg);
void pre_draw_call(eecs::Registry& reg);

//inline float get_game_dt(flecs::world& ecs) { return ecs.has<GamePaused>() ? 0.f : ecs.delta_time(); }
void set_last_level(const char* level);
