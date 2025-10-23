#include <flecs.h>
#include <raylib.h>

#include "math.h"
#include "movement.h"

struct math
{
  math(flecs::world& ecs)
  {
    ecs.module<math>();

    ecs.component<vec2i>()
      .member<int>("x")
      .member<int>("y");
    ecs.component<vec2f>()
      .member<float>("x")
      .member<float>("y");
    ecs.component<vec3f>()
      .member<float>("x")
      .member<float>("y")
      .member<float>("z");
    ecs.component<vec4f>()
      .member<float>("x")
      .member<float>("y")
      .member<float>("z")
      .member<float>("w");
    ecs.component<Position>()
      .member<vec3f>("val");
    ecs.component<Rotation>()
      .member<float>("val");
  }
};

void register_math(flecs::world& ecs)
{
  ecs.import<math>();
}

