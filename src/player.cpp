#include <raylib.h>
#include <eecs.h>

#include "player.h"
#include "tags.h"
#include "ui.h"
#include "level.h"
#include "game.h"
#include "audio.h"

void register_player(eecs::Registry& reg)
{
    static Sound stepSnd = LoadSound("res/audio/sfx/step_single_01.ogg");
    eecs::reg_system(reg, [&](eecs::EntityId eid, vec3f& position, vec3f& direction, Tag player)
    {
        if (is_ui_blocks_input())
            return;
        const float turnRight = (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_KP_7) ? -1.f : 0.f) +
                                (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_KP_9) ? 1.f  : 0.f);

        const vec3f rdir = vec3f{-direction.z, direction.y, direction.x};
        if (turnRight != 0.f)
            direction = rdir * turnRight;
        float forward = (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_KP_8) ? 1.f : 0.f) +
                        (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_KP_2) ? -1.f  : 0.f);
        float right = (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_KP_4) ? -1.f : 0.f) +
                      (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_KP_6) ? 1.f  : 0.f);
        if (forward != 0 && right != 0)
            right = 0.f;
        if (!eecs::has_comp(reg, eid, COMPID(Tag, ignoresCollision)))
        {
            vec3f mdir = direction * forward + right * rdir;
            vec2i mdir2d = vec2i(mdir.x, mdir.z);
            if (mdir2d.mag2() != 0)
            {
                vec2i gridPos = pos_to_grid(position);
                if (check_collision_dir(reg, gridPos, mdir2d))
                    return;
                // align back to grid
                PlaySound(stepSnd);
                position = vec3f(gridPos.x, 0.f, gridPos.y);
            }
        }
        position += direction * forward;
        position += right * rdir;
    }, COMPID(vec3f, position), COMPID(vec3f, direction), COMPID(const Tag, player));
}

