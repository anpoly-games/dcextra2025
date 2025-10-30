#include <eecs.h>
#include <raylib.h>
#include <reflection.h>

#include "../math.h"
#include "../tags.h"
#include "../ui.h"
#include "../cam.h"
#include "interactables.h"
#include "advancement.h"
#include "game_ui.h"

void draw_interactables(eecs::Registry& reg, float top, float scrwidth, float height, float scaleFactor)
{
    static NineRect nrect = create_9rect(LoadImage("res/textures/ui/button_rect.png"), 2);
    static Font actionFont = LoadFontEx("res/textures/ui/8px-IBM_BIOS_8x8.ttf", 8, nullptr, 0);
    eecs::query_entities(reg, [&](eecs::EntityId camEid, const Camera& camera)
    {
        const vec2i cam_wh = get_cam_wh(reg);
        vec2f pos = {cam_wh.x + 50.f * scaleFactor, top};
        eecs::query_entities(reg, [&](eecs::EntityId plEid, const vec3f& position, Tag player)
        {
            eecs::query_entities(reg, [&, &ppos = position](eecs::EntityId obj, const vec3f& position, const std::vector<eecs::EntityId>& actionList)
            {
                Vector3 pos2d = GetWorldToScreen3dEx(reg, toRLVec3(position), camera);
                if (pos2d.z < 0.f)
                    return;
                if (pos2d.x > cam_wh.x || pos2d.x < 0 || pos2d.y < 0 || pos2d.y > cam_wh.y)
                    return;
                const float step = 16.f * scaleFactor;
                const float width = scrwidth - pos.x - 4.f * 4.f * scaleFactor;
                //pos2d.x -= cam_wh.x * 0.5f;
                bool haveActions = false;
                for (eecs::EntityId act : actionList)
                {
                    eecs::query_components(reg, act, [&](float distance, const std::string& triggers, const std::string& text)
                    {
                        if ((ppos - position).mag2() > sqr(distance))
                            return;
                        haveActions = true;
                        std::string finalText = text;
                        eecs::query_component(reg, act, [&](const std::string& attribute)
                        {
                            std::string attrName = eecs::get_comp_or(reg, eecs::find_entity(reg, attribute.c_str()), COMPID(std::string, name), attribute);
                            const int attrVal = eecs::get_comp_or(reg, plEid, eecs::comp_id<int>(attribute.c_str()), 0);
                            std::string actionDifficultyMultName = triggers + "_difficultyMult";
                            const float mult = eecs::get_comp_or(reg, obj, eecs::comp_id<float>(actionDifficultyMultName.c_str()), 1.f);
                            const int chance = int(float(attrVal) * mult);
                            finalText += " (" + attrName + " " + std::to_string(chance) + "%)";
                        }, COMPID(const std::string, attribute));
                        draw_button_9rect(nrect, Rectangle(pos.x, pos.y, width, step), actionFont, finalText.c_str(), 8.f, 0, scaleFactor, ColorFromHSV(0, 0, 0.7f),
                        [&]()
                        {
                            eecs::emit_event(reg, fnv1StrHash(triggers.c_str()), obj, plEid);
                        });
                        //DrawLineEx({pos.x, pos.y + step * 0.5f}, {pos2d.x, pos2d.y}, 4.f, GetColor(0x63c74dff));
                        DrawLineEx({pos.x, pos.y + step * 0.5f}, {(float)cam_wh.x, pos.y + step * 0.5f}, 4.f, GetColor(0x63c74dff));
                        DrawLineEx({(float)cam_wh.x + 4.f, pos.y + step * 0.5f}, {pos2d.x, pos2d.y}, 4.f, GetColor(0x63c74dff));
                        pos.y += step + scaleFactor * 4.f;;
                    }, COMPID(const float, distance), COMPID(const std::string, triggers), COMPID(const std::string, text));
                }
                if (!haveActions)
                {
                }
            }, COMPID(const vec3f, position), COMPID(const std::vector<eecs::EntityId>, actionList));
        }, COMPID(const vec3f, position), COMPID(const Tag, player));
    }, COMPID(const Camera, camera));
}

void register_interactables(eecs::Registry& reg)
{
    static int textIndex = 0;
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
        eecs::query_component(reg, doorEid, [&](eecs::EntityId door_openSound)
        {
            eecs::query_component(reg, door_openSound, [&](const Sound& sound)
            {
                PlaySound(sound);
            }, COMPID(const Sound, sound));
        }, COMPID(const eecs::EntityId, door_openSound));
    }, COMPID(vec3f, position), COMPID(const vec3f, openRelPosition));

    eecs::create_entity_wrap(reg, "rolling_text")
        .set(COMPID(std::vector<ColoredText>, rollingText), {});
    eecs::on_event(reg, FNV1(hack), [&](eecs::EntityId doorEid, eecs::EntityId plEid, float hack_difficultyMult, int hack_successExperience)
    {
        eecs::query_component(reg, plEid, [&](int attr_mind)
        {
            const int dice = GetRandomValue(1, 100);
            const int attr = int(float(attr_mind) * hack_difficultyMult);
            bool success = dice < attr;
            push_rolling_text(reg, std::string(TextFormat("MIND (%d): roll %d vs %d\n", attr_mind, dice, attr)), success ? GetColor(0x3e8948ff) : GetColor(0xff0044ff));
            if (success)
            {
                eecs::emit_event(reg, FNV1(toggle), doorEid, plEid);
                add_exp(reg, plEid, hack_successExperience);
            }
            else
            {
                eecs::query_component(reg, doorEid, [&](eecs::EntityId hack_failureSound)
                {
                    eecs::query_component(reg, hack_failureSound, [&](const Sound& sound)
                    {
                        PlaySound(sound);
                    }, COMPID(const Sound, sound));
                }, COMPID(const eecs::EntityId, hack_failureSound));
                const int dmg = eecs::get_comp_or(reg, doorEid, COMPID(int, hack_failureDamage), 0);
                if (dmg > 0)
                {
                    eecs::query_component(reg, plEid, [&](int& hitpoints)
                    {
                        hitpoints = std::max(hitpoints - dmg, 0);
                    }, COMPID(int, hitpoints));
                    push_rolling_text(reg, std::string(TextFormat("Electrocuted for %d dmg", dmg)), GetColor(0xff0044ff));
                }
            }
        }, COMPID(const int, attr_mind));
    }, COMPID(const float, hack_difficultyMult), COMPID(const int, hack_successExperience));
}

