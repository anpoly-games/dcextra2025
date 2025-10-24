#include <raylib.h>
#include <eecs.h>

#include <filesystem>

#include "level.h"
#include "math.h"
#include "movement.h"
#include "tags.h"
#include "editor/editor_ui.h"
#include "game.h"
#include "prefabs.h"

namespace fs = std::filesystem;

bool check_collision_dir(eecs::Registry& reg, const vec2i& pos, const vec2i& dir)
{
    bool res = false;
    eecs::query_entities(reg, [&](eecs::EntityId, const std::set<vec2i>& level_xWalls, const std::set<vec2i>& level_zWalls)
    {
        // First figure out which walls to check
        const std::set<vec2i>& toCheck = dir.x != 0 ? level_xWalls : level_zWalls;
        const vec2i posCheck = vec2i{pos.x + (dir.x == 1 ? 1 : 0), pos.y + (dir.y == 1 ? 1 : 0)};
        res = toCheck.contains(posCheck);
    }, COMPID(const std::set<vec2i>, level_xWalls), COMPID(const std::set<vec2i>, level_zWalls));
    return res;
}


void register_level(eecs::Registry& reg)
{
    eecs::create_entity_wrap(reg, "level_walls")
        .set(COMPID(std::set<vec2i>, level_xWalls), {})
        .set(COMPID(std::set<vec2i>, level_zWalls), {});

    /*
    ecs_script_run_file(ecs, "res/prefabs/narrative.flecs");
    ecs_script_run_file(ecs, "res/prefabs/levels.flecs");
    */
}

void load_level(eecs::Registry& reg, const char* filename)
{
  std::filesystem::path fullPath{"res/levels/"};
  fullPath += filename;

  if (!fs::exists(fullPath))
    return;

  restart_world(reg);
  load_entities_from_file(reg, fullPath.string().c_str());

  set_last_level(filename);
}


