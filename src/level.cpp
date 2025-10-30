#include <raylib.h>
#include <raymath.h>
#include <eecs.h>
#include <climits>

#include <filesystem>

#include "level.h"
#include "math.h"
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

    std::vector<eecs::EntityId> floors = load_prefabs_from_file(reg, "res/prefabs/floors.edat");
    std::vector<eecs::EntityId> walls = load_prefabs_from_file(reg, "res/prefabs/walls.edat");
    std::vector<eecs::EntityId> doors = load_prefabs_from_file(reg, "res/prefabs/doors.edat");
    std::vector<eecs::EntityId> columns = load_prefabs_from_file(reg, "res/prefabs/columns.edat");
    std::vector<eecs::EntityId> ceilings = load_prefabs_from_file(reg, "res/prefabs/ceilings.edat");
    std::vector<eecs::EntityId> entities = load_prefabs_from_file(reg, "res/prefabs/entities.edat");
    std::vector<eecs::EntityId> logic = load_prefabs_from_file(reg, "res/prefabs/logic.edat");
    std::vector<eecs::EntityId> billboards = load_prefabs_from_file(reg, "res/prefabs/billboards.edat");

    eecs::create_entity_wrap(reg, "floors").set(COMPID(std::vector<eecs::EntityId>, children), floors);
    eecs::create_entity_wrap(reg, "walls").set(COMPID(std::vector<eecs::EntityId>, children), walls);
    eecs::create_entity_wrap(reg, "doors").set(COMPID(std::vector<eecs::EntityId>, children), doors);
    eecs::create_entity_wrap(reg, "columns").set(COMPID(std::vector<eecs::EntityId>, children), columns);
    eecs::create_entity_wrap(reg, "ceilings").set(COMPID(std::vector<eecs::EntityId>, children), ceilings);
    eecs::create_entity_wrap(reg, "entities").set(COMPID(std::vector<eecs::EntityId>, children), entities);
    eecs::create_entity_wrap(reg, "logic").set(COMPID(std::vector<eecs::EntityId>, children), logic);
    eecs::create_entity_wrap(reg, "billboards").set(COMPID(std::vector<eecs::EntityId>, children), billboards);

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
            if (int(position.y) != 0)
                return;
            std::set<vec2i>& toAdd = xzd.z == 0 ? level_zWalls : level_xWalls;
            toAdd.insert(xzd.xy());
        }, COMPID(std::set<vec2i>, level_xWalls), COMPID(std::set<vec2i>, level_zWalls));
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, wall));

    eecs::reg_enter(reg, [&](eecs::EntityId, const vec3f& position, float rotation, Tag door, Tag closed)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, std::set<vec2i>& level_xWalls, std::set<vec2i>& level_zWalls)
        {
            vec3i xzd = find_door_wall_coords(position, rotation);
            if (int(position.y) != 0)
                return;
            std::set<vec2i>& toAdd = xzd.z == 0 ? level_zWalls : level_xWalls;
            toAdd.insert(xzd.xy());
        }, COMPID(std::set<vec2i>, level_xWalls), COMPID(std::set<vec2i>, level_zWalls));
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, door), COMPID(const Tag, closed));

    eecs::reg_exit(reg, [&](eecs::EntityId, const vec3f& position, float rotation, Tag door, Tag closed)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, std::set<vec2i>& level_xWalls, std::set<vec2i>& level_zWalls)
        {
            vec3i xzd = find_door_wall_coords(position, rotation);
            if (int(position.y) != 0)
                return;
            std::set<vec2i>& toAdd = xzd.z == 0 ? level_zWalls : level_xWalls;
            toAdd.erase(xzd.xy());
        }, COMPID(std::set<vec2i>, level_xWalls), COMPID(std::set<vec2i>, level_zWalls));
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, door), COMPID(const Tag, closed));
}

void load_level(eecs::Registry& reg, const char* filename)
{
    std::filesystem::path fullPath{"res/levels/"};
    fullPath += filename;

    if (!fs::exists(fullPath))
        return;

    restart_world(reg);
    load_entities_from_file(reg, fullPath.string().c_str());
}

eecs::Registry* change_level(eecs::Registry& reg, std::unordered_map<std::string, eecs::Registry*>& registries)
{
    eecs::EntityId eid = eecs::find_entity(reg, "Switch_Level");
    if ( eid != eecs::invalid_eid )
    {
        std::string nextLevel{};
        eecs::query_component(reg, eid, [&](std::string& nl)
        {
            nextLevel = nl;
        }, COMPID(std::string, nextLevel));

        if ( !nextLevel.empty() )
        {
            eecs::del_entity(reg, eid);
            if ( registries.count(nextLevel) )
            {
                eecs::Registry* nReg = registries[nextLevel];
                assert(nReg);
                eecs::query_entities(*nReg, [&](eecs::EntityId eid, const vec3f& pos, const float rot, Tag spawn){
                    eecs::query_entities(*nReg, [&](eecs::EntityId pid, vec3f& position, vec3f& direction, vec3i& prevPosition, Tag player)
                    {
                        position = pos;
                        printf("pos = %f, %f, %f\n", pos.x, pos.y, pos.z);
                        direction = vec3f{sinf(rot*DEG2RAD), 0.f, cosf(rot*DEG2RAD)};
                        prevPosition = vec3i{INT_MAX, INT_MAX, INT_MAX};
                    }, COMPID(vec3f, position), COMPID(vec3f, direction), COMPID(vec3i, prevPosition), COMPID(Tag, player));
                },
                COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, spawn));
                return nReg;
            }

            std::filesystem::path fullPath{"res/levels/"};
            fullPath += nextLevel;

            if (!fs::exists(fullPath))
                return nullptr; // this is an error

            eecs::Registry* rNew = new eecs::Registry();
            if (eecs::find_entity(reg, "DebugMarker")!=eecs::invalid_eid)
            {
                eecs::EntityId eid = eecs::create_entity(*rNew, "DebugMarker"); // existence of this entity means we are in debug mode
            }
            restart_world(*rNew);

            load_entities_from_file(*rNew, fullPath.string().c_str());
            registries[nextLevel] = rNew;
            return rNew;
        }
        else
        {
            return &reg;
        }
    }

    return &reg;
}

bool ray_hit(eecs::Registry& reg, const vec3f& sourcePos, const vec3f& targetPos, eecs::EntityId target)
{
    bool res = false;
    vec3f dir = targetPos - sourcePos;
    const float initialDist = dir.mag();
    const float maxDistSq = sqr(initialDist + 1.f);
    dir = (1.f / initialDist) * dir;
    Ray r = {toRLVec3(sourcePos), toRLVec3(dir)};
    float bestT = 1e12f;
    eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, const Model& model)
    {
        if ((sourcePos - position).mag2() > maxDistSq || eid == target || bestT < initialDist)
            return;
        Matrix matRotation = MatrixRotate(Vector3{0.f, 1.f, 0.f}, eecs::get_comp_or(reg, eid, COMPID(float, rotation),0.f));
        Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

        Matrix matTransform = MatrixMultiply(matRotation, matTranslation);
        RayCollision coll = GetRayCollisionMesh(r, model.meshes[0], matTransform);
        if (!coll.hit)
            return;
        if (coll.distance < bestT)
            bestT = coll.distance;
    }, COMPID(const vec3f, position), COMPID(const Model, model));
    res = bestT < initialDist;
    return res;
}

bool check_occupancy(eecs::Registry& reg, const vec3f& pos)
{
    bool res = false;
    vec3i pos3d = pos_to_grid3d(pos);
    eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, Tag occupiesCell)
    {
        res |= pos_to_grid3d(position) == pos3d;
    }, COMPID(const vec3f, position), COMPID(const Tag, occupiesCell));
    return res;
}

