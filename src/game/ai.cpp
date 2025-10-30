#include <eecs.h>
#include <raylib.h>

#include "../math.h"
#include "../tags.h"
#include "../level.h"
#include "game_ui.h"
#include "ai.h"

void register_ai(eecs::Registry& reg)
{
    static Sound playerHit = LoadSound("res/audio/sfx/hit_07.ogg");
    eecs::on_event(reg, FNV1(next_turn), [&](eecs::EntityId eid, eecs::EntityId plEid, vec3f& position, int attackDamage, Tag ai)
    {
        eecs::query_components(reg, plEid, [&, &epos = position](const vec3f& position, int& hitpoints)
        {
            if (ray_hit(reg, epos, position + vec3f(0, 0.4f, 0), plEid))
                return;
            // try to move towards
            vec2f dirTo = position.xz() - epos.xz();
            if (dirTo.mag2() <= 1.f)
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
            if (!check_collision_dir(reg, pos_to_grid(epos), longMove))
                epos += vec3f(longMove.x, 0, longMove.y);
            else if (!check_collision_dir(reg, pos_to_grid(epos), shortMove))
                epos += vec3f(shortMove.x, 0, shortMove.y);
        }, COMPID(const vec3f, position), COMPID(int, hitpoints));
    }, COMPID(vec3f, position), COMPID(const int, attackDamage), COMPID(const Tag, ai));
}

