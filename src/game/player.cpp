#include <eecs.h>
#include <raylib.h>
#include <climits>

#include "../player.h"
#include "../math.h"
#include "../tags.h"

eecs::EntityId create_player(eecs::Registry& reg, float x, float z, float rot)
{
    eecs::EntityWrap player = eecs::create_entity_wrap(reg)
        .tag(COMPID(Tag, player))
        .set(COMPID(vec3f, position), {x, 0.f, z})
        .set(COMPID(vec3i, prevPosition), {INT_MAX, INT_MAX, INT_MAX})
        .set(COMPID(vec3f, direction), {sinf(rot*DEG2RAD), 0.f, cosf(rot*DEG2RAD)});
    return player.eid;
}

