#include <raylib.h>
#include <flecs.h>
#include <eecs.h>

#include <filesystem>

#include "level.h"
#include "math.h"
#include "movement.h"
#include "tags.h"
#include "editor/editor_ui.h"
#include "game.h"

namespace fs = std::filesystem;

struct level
{
  level(flecs::world& ecs)
  {
    ecs.module<level>();

    ecs.component<Wall>();
    ecs.component<Floor>();
    ecs.component<SpawnPoint>();

    ecs.component<Battle>();

    ecs.component<OpenOffset>()
      .member<vec3f>("val");

    ecs.component<OpenTime>()
      .member<float>("val");
    ecs.component<OpenProgress>()
      .member<float>("val");
    ecs.component<WishOpenPosition>()
      .member<float>("val");
    ecs.component<InitialPosition>()
      .member<vec3f>("val");

    ecs.component<TeleportOffset>()
      .member<vec3f>("val");

    ecs.component<Opened>();
    ecs.component<Closed>();
    ecs.component<Clickable>();
    ecs.component<Pickable>();
    ecs.component<Locked>();
    ecs.component<UnlockedBy>();

    ecs.component<Operational>();
    ecs.component<OpenOnOp>();
    ecs.component<LinkedBy>();
    ecs.component<RotateOnOp>().member<float>("val");

    ecs.component<DebugBox>()
      .member<vec3f>("color")
      .member<vec3f>("sz")
      .member<vec3f>("offset")
      .member<std::string>("name");

    ecs.component<Rotator>()
      .member<int>("val");

    ecs.component<TriggerVolume>()
      .member<vec2i>("sz")
      .member<std::string>("debugName");

    ecs.component<SwitchLevel>()
      .member<std::string>("levelName");

    ecs.component<HoverText>()
      .member<std::string>("text")
      .add(flecs::OnInstantiate, flecs::Inherit);

    ecs.entity("level_walls")
      .set(LevelWalls());


    ecs.component<NarrativeLine>()
      .member<std::string>("text");
    ecs.component<BlockingDialog>();
    ecs.component<SpawnDialog>();
    ecs.component<IconStatic>().member<vec2f>("val");

    ecs.component<OperateOnRemove>();

    ecs.component<TreasureItem>();

    ecs.component<GrantsItem>();

    ecs.component<Color>()
      .member<unsigned char>("r")
      .member<unsigned char>("g")
      .member<unsigned char>("b")
      .member<unsigned char>("a");

    ecs.component<ParticleColor>()
      .member<vec4f>("val");
    ecs.component<NumParticles>()
      .member<int>("val");
    ecs.component<ParticleSize>()
      .member<float>("val");

    // ecs.component<GateOpenOp>();

    static auto find_door_wall_coords = [](const Position& pos, const Rotation& rot)
    {
      const int dir = rot.val == 0 || rot.val == 180 ? 0 : 1;
      const int x = floorf((pos.x + (dir ? 0.5f : 0.f)) + 0.5f);
      const int z = floorf((pos.z + (dir ? 0.f : 0.5f)) + 0.5f);
      return vec3i{x, z, dir};
    };

    ecs.observer<const Position, const Rotation>()
      .with<Wall>()
      .event(flecs::OnSet)
      .each([&](const Position& pos, const Rotation& rot)
      {
        auto lwQ = ecs.query<LevelWalls>();
        lwQ.each([&](LevelWalls& lw)
        {
          vec3i xzd = find_door_wall_coords(pos, rot);
          std::set<vec2i>& toAdd = xzd.z == 0 ? lw.zWalls : lw.xWalls;
          toAdd.insert(xzd.xy());
        });
      });
    ecs.observer<const Position, const Rotation>()
      .with<Door>()
      .with<Closed>()
      .event(flecs::OnSet)
      .each([&](const Position& pos, const Rotation& rot)
      {
        auto lwQ = ecs.query<LevelWalls>();
        lwQ.each([&](LevelWalls& lw)
        {
          vec3i xzd = find_door_wall_coords(pos, rot);
          std::set<vec2i>& toAdd = xzd.z == 0 ? lw.zWalls : lw.xWalls;
          toAdd.insert(xzd.xy());
        });
      });
    ecs.observer<const Position, const Rotation>()
      .with<Door>()
      .with<Closed>()
      .event(flecs::OnRemove)
      .each([&](const Position& pos, const Rotation& rot)
      {
        auto lwQ = ecs.query<LevelWalls>();
        lwQ.each([&](LevelWalls& lw)
        {
          vec3i xzd = find_door_wall_coords(pos, rot);
          std::set<vec2i>& toAdd = xzd.z == 0 ? lw.zWalls : lw.xWalls;
          toAdd.erase(xzd.xy());
        });
      });

    ecs.system<DisappearTimer>()
      .each([&](flecs::entity e, DisappearTimer& timer)
      {
        timer.x += ecs.delta_time();
        if (timer.x >= timer.y)
        {
          e.children([&](flecs::entity chi)
          {
            chi.destruct();
          });
          e.destruct();
        }
      });
  }
};

static std::string savedState = "";

bool check_collision_dir(flecs::world& ecs, const vec2i& pos, const vec2i& dir)
{
  bool res = false;
  auto lwQ = ecs.query<const LevelWalls>();
  lwQ.each([&](const LevelWalls& lw)
  {
    // First figure out which walls to check
    const std::set<vec2i>& toCheck = dir.x != 0 ? lw.xWalls : lw.zWalls;
    const vec2i posCheck = vec2i{pos.x + (dir.x == 1 ? 1 : 0), pos.y + (dir.y == 1 ? 1 : 0)};
    res = toCheck.contains(posCheck);
  });
  return res;
}


void register_level(flecs::world& ecs)
{
  ecs.import<level>();

  ecs_script_run_file(ecs, "res/prefabs/narrative.flecs");
  ecs_script_run_file(ecs, "res/prefabs/levels.flecs");
}


template<typename Callable>
void parse_lines(const std::string& str, Callable c)
{
  size_t start = 0;
  size_t next = str.find_first_of("\n", start);
  do
  {
    if (next == std::string::npos)
    {
      c(str.substr(start));
      break;
    }
    c(str.substr(start, next - start));
    start = next + 1;
    next = str.find_first_of("\n", start);
  } while (true);
}

static void load_from_str(eecs::Registry& reg, flecs::world& ecs, const std::string& jsonish)
{
  restart_world(reg, ecs);
  parse_lines(jsonish, [&](const std::string& substr)
  {
    if (substr.empty())
      return;
    flecs::entity e = ecs.entity();
    e.from_json(substr.c_str());
    flecs::entity pref = e.target(flecs::IsA);
    e.remove(flecs::IsA, pref);
    e.is_a(pref);
  });
}

void load_level(eecs::Registry& reg, flecs::world& ecs, const char* filename)
{
  std::filesystem::path fullPath{"res/levels/"};
  fullPath += filename;
  if (!fs::exists(fullPath))
    return;
  auto fileSz = fs::file_size(fullPath);
  FILE* f = fopen(fullPath.string().c_str(), "rb");
  if (!f)
    return;

  std::string jsonish;
  jsonish.resize(fileSz);
  size_t offs = 0;
  size_t rpos = fread(jsonish.data(), 1, fileSz, f);
  while (rpos < fileSz)
    rpos += fread(jsonish.data() + offs, 1, fileSz - offs, f);
  fclose(f);

  load_from_str(reg, ecs, jsonish);
  set_last_level(filename);
}

  /*
void load_state(flecs::world& ecs)
{
  if (savedState.empty())
    return;

  printf("nolo: %s\n", savedState.c_str());
  load_from_str(ecs, savedState);
}
  */

void save_state(flecs::world& ecs)
{
  std::string jsonish = "";
  auto q = ecs.query_builder().with<Saveable>().build();
  q.each([&](flecs::entity e)
  {
    jsonish += e.to_json();
    jsonish += "\n";
  });
  if (!jsonish.empty())
    jsonish.resize(jsonish.size() - 1);
  savedState = jsonish;
}


