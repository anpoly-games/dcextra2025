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
        eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, const vec2i& trigger_volume)
        {
            vec3i triggerPos = pos_to_grid3d(position);
            vec2i volExt = vec2i((trigger_volume.x - 1) / 2, (trigger_volume.y - 1) / 2);
            if (triggerPos.y == curPosition.y &&
                curPosition.x >= triggerPos.x - volExt.x && curPosition.x <= triggerPos.x + volExt.x &&
                curPosition.z >= triggerPos.z - volExt.y && curPosition.z <= triggerPos.z + volExt.y)
            {
                eecs::emit_event(reg, FNV1(enterTrigger), eid, playerEid);
            }
        }, COMPID(const vec3f, position), COMPID(const vec2i, trigger_volume));
    }, COMPID(vec3i, prevPosition), COMPID(const vec3f, position));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId, eecs::EntityId, const std::string& level_switchTo)
    {
        load_level(reg, level_switchTo.c_str());
    }, COMPID(const std::string, level_switchTo));
}

