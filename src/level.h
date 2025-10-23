#pragma once

#include <string>
#include <set>
#include <vector>
#include "editor/editor_ui.h"
#include "math.h"
#include "podval.h"

void register_level(flecs::world& ecs);

void load_level(eecs::Registry& reg, flecs::world& ecs, const char* filename);
void load_state(flecs::world& ecs);
void save_state(flecs::world& ecs);

struct OpenOffset : public vec3f { ASSIGN_OP_3f(OpenOffset); };
struct OpenTime : public FVal {};
struct OpenProgress : public FVal {};
struct WishOpenPosition : public FVal {};
struct InitialPosition : public vec3f { ASSIGN_OP_3f(InitialPosition); };

struct DebugBox
{
  vec3f color;
  vec3f sz;
  vec3f offset;
  std::string name;
};

struct TriggerVolume
{
  vec2i sz;
  std::string debugName;
};

struct Operational {};
struct OpenOnOp {};
struct LinkedBy {};

struct SwitchLevel
{
  std::string name;
};

struct SpawnPoint {};

struct LevelWalls
{
  std::set<vec2i> xWalls;
  std::set<vec2i> zWalls;
};

struct HoverText
{
  std::string text;
};

struct Rotator
{
  int val;
};

struct NarrativeLine
{
  std::string text;
};
struct BlockingDialog {};
struct DisappearTimer : public vec2f {};
struct SpawnDialog {};
struct IconStatic : public vec2f {};

struct OperateOnRemove {};

struct RotateOnOp : public FVal {};

struct TeleportOffset : public vec3f { ASSIGN_OP_3f(TeleportOffset); };

struct Battle {};
struct TreasureItem {};
struct TreasureCounter: public IVal {};
struct GrantsItem {};

struct ParticleColor : public vec4f {};
struct NumParticles : public IVal {};
struct ParticleSize : public FVal {};

struct Particles
{
  std::vector<vec3f> particles;
};

//struct GateOpenOp {};

// Only one step at a time, not a visibility check!
bool check_collision_dir(flecs::world& ecs, const vec2i& pos, const vec2i& dir);

