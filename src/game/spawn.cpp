#include <eecs.h>
#include <raylib.h>

#include "../math.h"
#include "../tags.h"
#include "../player.h"
#include "../cam.h"
#include "spawn.h"

void register_spawn(eecs::Registry& reg)
{
    eecs::reg_enter(reg, [&](eecs::EntityId, const vec3f& position, float rotation, Tag spawn)
    {
        bool hasPlayer = false;
        eecs::query_entities(reg, [&, &spawnPos = position](eecs::EntityId, vec3f& position, vec3f& direction, Tag player)
        {
            hasPlayer = true;
            position = spawnPos;
            direction = vec3f(sinf(rotation * DEG2RAD), 0.f, cosf(rotation * DEG2RAD));
        }, COMPID(vec3f, position), COMPID(vec3f, direction), COMPID(const Tag, player));
        if (!hasPlayer)
        {
            create_player(reg, position.x, position.z, rotation);
            create_cam(reg);
        }
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, spawn));
}

