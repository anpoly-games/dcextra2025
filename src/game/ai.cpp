#include <eecs.h>
#include <raylib.h>

#include "dcengine/math.h"
#include "dcengine/tags.h"
#include "dcengine/level.h"
#include "game_ui.h"
#include "ai.h"

void register_ai(eecs::Registry& reg)
{
    static Sound playerHit = LoadSound("res/audio/sfx/hit_07.ogg");
    eecs::on_event(reg, FNV1(next_turn), [&](eecs::EntityId eid, eecs::EntityId plEid, vec3f& position, int attackDamage, Tag ai, int aggroesTeam)
    {
        eecs::query_components(reg, plEid, [&, &epos = position](const vec3f& position, int& hitpoints, int team)
        {
            if (aggroesTeam != team)
                return;
            if (ray_hit(reg, epos, position + vec3f(0, 0.4f, 0), plEid))
                return;
            // try to move towards
            vec3f dir3d = position - epos;
            vec2f dirTo = dir3d.xz();
            if (dirTo.mag2() <= 1.1f && fabsf(dir3d.y) < 1.f)
            {
                // Attack player!
                hitpoints = std::max(hitpoints - attackDamage, 0);
                push_rolling_text(reg, TextFormat("Enemy deals you %d dmg", attackDamage), GetColor(0xff0044ff));
                PlaySound(playerHit);
                return;
            }
            // first try to move along longest one
            vec2i longMove;
            vec2i shortMove;
            if (fabsf(dirTo.x) > fabsf(dirTo.y))
            {
                longMove = vec2i(sign(dirTo.x), 0);
                shortMove = vec2i(0, sign(dirTo.y));
            }
            else
            {
                longMove = vec2i(0, sign(dirTo.y));
                shortMove = vec2i(sign(dirTo.x), 0);
            }
            const vec3f lmove3d = vec3f(longMove.x, 0, longMove.y);
            const vec3f smove3d = vec3f(shortMove.x, 0, shortMove.y);
            if (!check_collision_dir(reg, epos, longMove) && check_floor(reg, epos + lmove3d) && !check_occupancy(reg, epos + lmove3d))
                epos += vec3f(longMove.x, 0, longMove.y);
            else if (!check_collision_dir(reg, epos, shortMove) && check_floor(reg, epos + smove3d) && !check_occupancy(reg, epos + smove3d))
                epos += vec3f(shortMove.x, 0, shortMove.y);
        }, COMPID(const vec3f, position), COMPID(int, hitpoints), COMPID(const int, team));
    }, COMPID(vec3f, position), COMPID(const int, attackDamage), COMPID(const Tag, ai), COMPID(const int, aggroesTeam));
}

