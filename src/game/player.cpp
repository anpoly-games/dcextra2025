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
        .set(COMPID(vec3f, direction), {sinf(rot*DEG2RAD), 0.f, cosf(rot*DEG2RAD)})
        .set(COMPID(int, attr_strength), 50)
        .set(COMPID(int, attr_agility), 50)
        .set(COMPID(int, attr_mind), 50)
        .set(COMPID(int, attr_body), 50);
    eecs::create_entity_wrap(reg, "attr_strength")
        .set(COMPID(std::string, name), {"STR"});
    eecs::create_entity_wrap(reg, "attr_agility")
        .set(COMPID(std::string, name), {"AGI"});
    eecs::create_entity_wrap(reg, "attr_mind")
        .set(COMPID(std::string, name), {"MIND"});
    eecs::create_entity_wrap(reg, "attr_body")
        .set(COMPID(std::string, name), {"BODY"});
    return player.eid;
}

