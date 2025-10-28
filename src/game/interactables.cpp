#include <eecs.h>
#include <raylib.h>
#include <reflection.h>

#include "../math.h"
#include "../tags.h"
#include "../ui.h"
#include "../cam.h"
#include "interactables.h"

void draw_interactables(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    static NineRect nrect = create_9rect(LoadImage("res/textures/ui/button_rect.png"), 2);
    eecs::query_entities(reg, [&](eecs::EntityId, const Camera& camera)
    {
        eecs::query_entities(reg, [&](eecs::EntityId plEid, const vec3f& position, Tag player)
        {
            eecs::query_entities(reg, [&, &ppos = position](eecs::EntityId obj, const vec3f& position, const std::vector<eecs::EntityId>& actionList)
            {
                Vector3 pos2d = GetWorldToScreen3dEx(toRLVec3(position), camera);
                if (pos2d.z < 0.f)
                    return;
                const float step = 20.f;
                const float width = step * 10.f;
                pos2d.x -= width * 0.5f;
                for (eecs::EntityId act : actionList)
                {
                    eecs::query_components(reg, act, [&](float distance, const std::string& triggers, const std::string& text)
                    {
                        draw_button_9rect(nrect, Rectangle(pos2d.x, pos2d.y, width, step), GetFontDefault(), text.c_str(), 12.f, scaleFactor, WHITE,
                        [&]()
                        {
                            eecs::emit_event(reg, fnv1StrHash(triggers.c_str()), obj, plEid);
                        });
                        pos2d.y += step;
                    }, COMPID(const float, distance), COMPID(const std::string, triggers), COMPID(const std::string, text));
                }
            }, COMPID(const vec3f, position), COMPID(const std::vector<eecs::EntityId>, actionList));
        }, COMPID(const vec3f, position), COMPID(const Tag, player));
    }, COMPID(const Camera, camera));
}

void register_interactables(eecs::Registry& reg)
{
    eecs::on_event(reg, FNV1(toggle), [&](eecs::EntityId doorEid, eecs::EntityId plEid, vec3f& position, const vec3f& openRelPosition)
    {
        // NOTE: Order is important, we need to remove closed tag before moving, so on_exit will trigger at appropriate position!!!
        // Same for closing, it should be moved into an original position and then tag should be set
        if (eecs::has_comp(reg, doorEid, COMPID(Tag, closed)))
        {
            eecs::del_component(reg, doorEid, COMPID(Tag, closed));
            position += openRelPosition;
        }
        else
        {
            position -= openRelPosition;
            eecs::set_component(reg, doorEid, COMPID(Tag, closed), Tag{});
        }
    }, COMPID(vec3f, position), COMPID(const vec3f, openRelPosition));

    eecs::on_event(reg, FNV1(hack), [&](eecs::EntityId doorEid, eecs::EntityId plEid, vec3f& position, const vec3f& openRelPosition, float hack_difficultyMult)
    {
        eecs::query_component(reg, plEid, [&](int attr_mind)
        {
            const int dice = GetRandomValue(1, 100);
            const int attr = int(float(attr_mind) * hack_difficultyMult);
            if (dice < attr)
            {
                eecs::del_component(reg, doorEid, COMPID(Tag, closed));
                position += openRelPosition;
            }
            // TODO: show dice roll
            printf("mind (%d): roll %d vs %d\n", attr_mind, dice, attr);
        }, COMPID(const int, attr_mind));
    }, COMPID(vec3f, position), COMPID(const vec3f, openRelPosition), COMPID(const float, hack_difficultyMult));
}

