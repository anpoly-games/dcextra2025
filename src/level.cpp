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

    std::vector<eecs::EntityId> walls = load_prefabs_from_file(reg, "res/prefabs/walls.edat");
    eecs::create_entity_wrap(reg, "floors");

    eecs::create_entity_wrap(reg, "walls")
        .set(COMPID(std::vector<eecs::EntityId>, children), walls);

    eecs::create_entity_wrap(reg, "doors");
    eecs::create_entity_wrap(reg, "columns");
    eecs::create_entity_wrap(reg, "ceilings");
    eecs::create_entity_wrap(reg, "entities");
    eecs::create_entity_wrap(reg, "logic");

    static auto find_door_wall_coords = [](const vec3f& pos, float rot)
    {
        const int dir = rot == 0 || rot == 180 ? 0 : 1;
        const int x = floorf((pos.x + (dir ? 0.5f : 0.f)) + 0.5f);
        const int z = floorf((pos.z + (dir ? 0.f : 0.5f)) + 0.5f);
        return vec3i{x, z, dir};
    };
    eecs::reg_enter(reg, [&](eecs::EntityId, const vec3f& position, float rotation, Tag wall)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, std::set<vec2i>& level_xWalls, std::set<vec2i>& level_zWalls)
        {
            vec3i xzd = find_door_wall_coords(position, rotation);
            std::set<vec2i>& toAdd = xzd.z == 0 ? level_zWalls : level_xWalls;
            toAdd.insert(xzd.xy());
        }, COMPID(std::set<vec2i>, level_xWalls), COMPID(std::set<vec2i>, level_zWalls));
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, wall));
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


