#include <eecs.h>
#include <raylib.h>

#include "../math.h"
#include "../tags.h"
#include "../level.h"
#include "triggers.h"


void register_triggers(eecs::Registry& reg)
{
    eecs::reg_system(reg, [&](eecs::EntityId playerEid, vec3i& prevPosition, const vec3f& position)
    {
        vec3i curPosition = pos_to_grid3d(position);
        if (curPosition == prevPosition)
            return;
        prevPosition = curPosition;
        eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, const vec2i& trigger_volume)
        {
            vec3i triggerPos = pos_to_grid3d(position);
            vec2i volExt = vec2i((trigger_volume.x - 1) / 2, (trigger_volume.y - 1) / 2);
            if (triggerPos.y == curPosition.y &&
                curPosition.x >= triggerPos.x - volExt.x && curPosition.x <= triggerPos.x + volExt.x &&
                curPosition.z >= triggerPos.z - volExt.y && curPosition.z <= triggerPos.z + volExt.y)
            {
                printf("emmiting event %i\n", FNV1(enterTrigger));
                eecs::emit_event(reg, FNV1(enterTrigger), eid, playerEid);
            }
        }, COMPID(const vec3f, position), COMPID(const vec2i, trigger_volume));
    }, COMPID(vec3i, prevPosition), COMPID(const vec3f, position));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId, eecs::EntityId, const std::string& level_switchTo)
    {
        printf("switching to level %s\n", level_switchTo.c_str());
        //load_level(reg, level_switchTo.c_str());
        //change_level(reg, level_switchTo.c_str());
        eecs::create_or_find_entity_wrap(reg, "Switch_Level").set(COMPID(std::string, nextLevel), level_switchTo);
    }, COMPID(const std::string, level_switchTo));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId eid, eecs::EntityId playerId, const vec2f& displacement, Tag teleporter)
    {
        eecs::query_components(reg, playerId, [&](vec3f& position, Tag player)
        {
            position.x += displacement.x;
            position.z += displacement.y; // displacement is in the x-z plane
        }, COMPID(vec3f, position), COMPID(Tag, player));
    }, COMPID(vec2f, displacement), COMPID(Tag, teleporter));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId eid, eecs::EntityId playerId, const float rel_displacement, Tag relative_teleporter)
    {
        eecs::query_components(reg, playerId, [&](vec3f& position, const vec3f& direction, Tag player)
        {
            position = position + rel_displacement * direction;
        }, COMPID(vec3f, position), COMPID(const vec3f, direction), COMPID(Tag, player));
    }, COMPID(float, rel_displacement), COMPID(Tag, relative_teleporter));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId eid, eecs::EntityId playerId, const int turns, Tag spinner)
    {
        eecs::query_components(reg, playerId, [&](vec3f& direction, Tag player)
        {
            if (turns==2)
            {
                direction.x = -direction.x;
                direction.z = -direction.z;
            }
            else
            {
                direction = vec3{-turns * direction.z, direction.y, turns * direction.x};
            }
        }, COMPID(vec3f, direction), COMPID(Tag, player));
    }, COMPID(const int, turns), COMPID(Tag, spinner));
}

