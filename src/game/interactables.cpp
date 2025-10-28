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
        eecs::query_entities(reg, [&](eecs::EntityId, const vec3f& position, Tag player)
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
                        draw_button_9rect(nrect, Rectangle(pos2d.x, pos2d.y, width, step), GetFontDefault(), text.c_str(), 12.f, scaleFactor, WHITE);
                        pos2d.y += step;
                    }, COMPID(const float, distance), COMPID(const std::string, triggers), COMPID(const std::string, text));
                }
            }, COMPID(const vec3f, position), COMPID(const std::vector<eecs::EntityId>, actionList));
        }, COMPID(const vec3f, position), COMPID(const Tag, player));
    }, COMPID(const Camera, camera));
}

