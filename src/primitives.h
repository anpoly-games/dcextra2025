#pragma once

#include "math.h"
#include <string>

void register_primitives(flecs::world& ecs);

struct Cube
{
  vec3f sz;
};

struct Billboard
{
  vec2f sz;
};

struct ModelFile
{
  std::string filename;
};

struct RelativePos : public vec3f {};

