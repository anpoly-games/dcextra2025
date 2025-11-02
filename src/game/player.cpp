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
        .set(COMPID(int, team), 0)
        .set(COMPID(int, attr_strength), 25)
        .set(COMPID(int, attr_agility), 25)
        .set(COMPID(int, attr_mind), 25)
        .set(COMPID(int, attr_body), 50)
        .set(COMPID(int, prevHitpoints), 50)
        .set(COMPID(int, hitpoints), 50)
        .set(COMPID(int, level), 1)
        .set(COMPID(int, pointsToSpend), 50)
        .set(COMPID(int, experience), 0)
        .set(COMPID(int, items_regenX), 0)
        .set(COMPID(int, items_bearserker), 0)
        .set(COMPID(int, items_reflexxx), 0)
        .set(COMPID(int, items_mindDefoger), 0)
        .set(COMPID(int, items_genius), 0)
        .set(COMPID(int, items_bandito), 0)
        .set(COMPID(float, timeSinceHit), 10.f);
    eecs::create_entity_wrap(reg, "attr_strength")
        .set(COMPID(std::string, name), {"STR"});
    eecs::create_entity_wrap(reg, "attr_agility")
        .set(COMPID(std::string, name), {"AGI"});
    eecs::create_entity_wrap(reg, "attr_mind")
        .set(COMPID(std::string, name), {"MIND"});
    eecs::create_entity_wrap(reg, "attr_body")
        .set(COMPID(std::string, name), {"BODY"});
    eecs::create_entity_wrap(reg)
        .set(COMPID(float, globalCd), -1.f)
        .set(COMPID(float, cooldownSplit), 0.2f);
    return player.eid;
}

