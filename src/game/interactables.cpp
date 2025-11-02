#include <eecs.h>
#include <raylib.h>
#include <raymath.h>
#include <reflection.h>

#include "../math.h"
#include "../tags.h"
#include "../ui.h"
#include "../cam.h"
#include "../level.h"
#include "../game.h"
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
        vec2f initialPos = pos;
        const float step = 16.f * scaleFactor;
        const float width = scrwidth - pos.x - 4.f * 4.f * scaleFactor;
        eecs::query_entities(reg, [&](eecs::EntityId plEid, const vec3f& position, Tag player, int team)
        {
            eecs::query_entities(reg, [&, &ppos = position](eecs::EntityId obj, const vec3f& position, const std::vector<eecs::EntityId>& actionList)
            {
                vec3f flPos = vec3f(position.x, floorf(position.y), position.z);
                Vector3 pos2d = GetWorldToScreen3dEx(reg, toRLVec3(position), camera);
                if (pos2d.z < 0.f)
                    return;
                if (pos2d.x > cam_wh.x || pos2d.x < 0 || pos2d.y < 0 || pos2d.y > cam_wh.y)
                    return;
                bool shouldSkip = false;
                eecs::query_components(reg, obj, [&](int aggroesTeam)
                {
                    shouldSkip = team != aggroesTeam;
                }, COMPID(const int, aggroesTeam));
                if (shouldSkip)
                    return;
                //pos2d.x -= cam_wh.x * 0.5f;
                bool haveActions = false;
                for (eecs::EntityId act : actionList)
                {
                    eecs::query_components(reg, act, [&](float distance, const std::string& triggers, const std::string& text)
                    {
                        if ((ppos - flPos).mag2() > sqr(distance) || ray_hit(reg, ppos + vec3f(0.f, 0.4f, 0.f), position, obj))
                            return;
                        haveActions = true;
                        std::string finalText = text;
                        eecs::query_component(reg, act, [&](const std::string& attribute)
                        {
                            const float attrMult = eecs::get_comp_or(reg, act, COMPID(float, attribute_modifier), 1.f);
                            std::string attrName = eecs::get_comp_or(reg, eecs::find_entity(reg, attribute.c_str()), COMPID(std::string, name), attribute);
                            const int attrVal = eecs::get_comp_or(reg, plEid, eecs::comp_id<int>(attribute.c_str()), 0);
                            std::string actionDifficultyMultName = triggers + "_difficultyMult";
                            const float mult = eecs::get_comp_or(reg, obj, eecs::comp_id<float>(actionDifficultyMultName.c_str()), 1.f);
                            const float genMult = eecs::has_comp(reg, plEid, COMPID(Tag, genius)) ? 2.f : 1.f;
                            const int chance = int(float(attrVal) * mult * attrMult * genMult);
                            finalText += " (" + attrName + " " + std::to_string(chance) + "%)";
                        }, COMPID(const std::string, attribute));
                        draw_button_9rect(nrect, Rectangle(pos.x, pos.y, width, step), actionFont, finalText.c_str(), 8.f, 0, scaleFactor, ColorFromHSV(0, 0, 0.7f),
                        [&]()
                        {
                            eecs::query_entities(reg, [&](eecs::EntityId, float& globalCd, float cooldownSplit)
                            {
                                if (is_ui_blocks_input(reg) || globalCd < cooldownSplit)
                                    return;
                                eecs::emit_event(reg, fnv1StrHash(triggers.c_str()), act, plEid);
                                globalCd = 0.f;
                            }, COMPID(float, globalCd), COMPID(const float, cooldownSplit));
                        });
                        //DrawLineEx({pos.x, pos.y + step * 0.5f}, {pos2d.x, pos2d.y}, 4.f, GetColor(0x63c74dff));
                        DrawLineEx({pos.x, pos.y + step * 0.5f}, {(float)cam_wh.x, pos.y + step * 0.5f}, 4.f, GetColor(0x63c74dff));
                        DrawLineEx({(float)cam_wh.x + 4.f, pos.y + step * 0.5f}, {pos2d.x, pos2d.y}, 4.f, GetColor(0x63c74dff));
                        pos.y += step + scaleFactor * 4.f;
                    }, COMPID(const float, distance), COMPID(const std::string, triggers), COMPID(const std::string, text));
                }
                if (!haveActions)
                {
                }
            }, COMPID(const vec3f, position), COMPID(const std::vector<eecs::EntityId>, actionList));
            draw_button_9rect(nrect, Rectangle(initialPos.x, initialPos.y + (step + scaleFactor * 4.f) * 7, width, step), actionFont, "Skip turn", 8.f, 0, scaleFactor, ColorFromHSV(0, 0, 0.7f),
            [&]()
            {
                eecs::emit_event(reg, FNV1(next_turn), eecs::invalid_eid, plEid);
            });
        }, COMPID(const vec3f, position), COMPID(const Tag, player), COMPID(const int, team));
    }, COMPID(const Camera, camera));
}

void register_interactables(eecs::Registry& reg)
{
    static int textIndex = 0;
    static Sound playerHit = LoadSound("res/audio/sfx/hit_07.ogg");
    eecs::reg_enter(reg, [&](eecs::EntityId eid, const std::vector<std::string>& actionPrefabs)
    {
        std::vector<eecs::EntityId> actionList;
        for (const std::string& prefabName : actionPrefabs)
        {
            eecs::EntityWrap action = eecs::create_wrap_from_prefab(reg, eecs::find_entity(reg, prefabName.c_str()))
                .set(COMPID(eecs::EntityId, parent), eid);
            actionList.emplace_back(action.eid);
        }
        eecs::set_component(reg, eid, COMPID(std::vector<eecs::EntityId>, actionList), actionList);
    }, COMPID(const std::vector<std::string>, actionPrefabs));
    eecs::on_event(reg, FNV1(switch_level), [&](eecs::EntityId actEid, eecs::EntityId plEid, const std::string& level_name)
    {
        eecs::create_or_find_entity_wrap(reg, "Switch_Level").set(COMPID(std::string, nextLevel), level_name);
    }, COMPID(const std::string, level_name));
    eecs::on_event(reg, FNV1(toggle), [&](eecs::EntityId actEid, eecs::EntityId plEid, eecs::EntityId parent)
    {
        eecs::emit_event(reg, FNV1(open), parent, plEid);
    }, COMPID(const eecs::EntityId, parent));
    eecs::on_event(reg, FNV1(open), [&](eecs::EntityId doorEid, eecs::EntityId plEid, vec3f& position, const vec3f& openRelPosition)
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
    auto attrRoll = [](eecs::Registry& reg, const char* attrName, int attrVal, float diffMult) -> bool
    {
        const int dice = GetRandomValue(1, 100);
        const int attr = int(float(attrVal) * diffMult);
        const bool success = dice < attr;
        push_rolling_text(reg, TextFormat("%s (%d): roll %d vs %d\n", attrName, attrVal, dice, attr), success ? GetColor(0x3e8948ff) : GetColor(0xff0044ff));
        return success;
    };
    auto procSkillcheck = [attrRoll](eecs::Registry& reg, const char* attrName, int attrVal, float diffMult,
            eecs::EntityId actEid, int action_experience,
            fnv1_hash_t succEvt, eecs::EntityId target, eecs::EntityId plEid)
    {
        if (eecs::has_comp(reg, plEid, COMPID(Tag, genius)))
        {
            diffMult *= 2.f;
            push_rolling_text(reg, "GeniusInj: wears out", WHITE);
            eecs::del_component(reg, plEid, COMPID(Tag, genius));
        }
        if (attrRoll(reg, attrName, attrVal, diffMult))
        {
            eecs::emit_event(reg, succEvt, target, plEid);
            add_exp(reg, plEid, action_experience);
        }
        else
        {
            eecs::query_component(reg, actEid, [&](eecs::EntityId action_failSound)
            {
                eecs::query_component(reg, action_failSound, [&](const Sound& sound)
                {
                    PlaySound(sound);
                }, COMPID(const Sound, sound));
            }, COMPID(const eecs::EntityId, action_failSound));
            eecs::query_components(reg, actEid, [&](int action_failDamage, const std::string& action_failMsg)
            {
                eecs::query_component(reg, plEid, [&](int& hitpoints)
                {
                    PlaySound(playerHit);
                    hitpoints = std::max(hitpoints - action_failDamage, 0);
                }, COMPID(int, hitpoints));
                push_rolling_text(reg, TextFormat(action_failMsg.c_str(), action_failDamage), GetColor(0xff0044ff));
            }, COMPID(const int, action_failDamage), COMPID(const std::string, action_failMsg));
        }
    };
    eecs::on_event(reg, FNV1(hack), [&](eecs::EntityId actEid, eecs::EntityId plEid, eecs::EntityId parent, int action_experience, const std::string& action_successEvent)
    {
        const float attrMult = eecs::get_comp_or(reg, actEid, COMPID(float, attribute_modifier), 1.f);
        eecs::query_component(reg, plEid, [&](int attr_mind)
        {
            procSkillcheck(reg, "MIND", attr_mind, attrMult, actEid, action_experience, fnv1StrHash(action_successEvent.c_str()), parent, plEid);
        }, COMPID(const int, attr_mind));
    }, COMPID(const eecs::EntityId, parent), COMPID(const int, action_experience), COMPID(const std::string, action_successEvent));

    eecs::on_event(reg, FNV1(shove), [&](eecs::EntityId actEid, eecs::EntityId plEid, eecs::EntityId parent, const std::string& attribute, int shove_distance)
    {
        float attrMult = eecs::get_comp_or(reg, actEid, COMPID(float, attribute_modifier), 1.f);
        if (eecs::has_comp(reg, plEid, COMPID(Tag, genius)))
        {
            attrMult *= 2.f;
            push_rolling_text(reg, "GeniusInj: wears out", WHITE);
            eecs::del_component(reg, plEid, COMPID(Tag, genius));
        }
        eecs::query_components(reg, plEid, [&](int attr, const vec3f& position)
        {
            std::string attrName = eecs::get_comp_or(reg, eecs::find_entity(reg, attribute.c_str()), COMPID(std::string, name), attribute);
            if (attrRoll(reg, attrName.c_str(), attr, attrMult))
            {
                eecs::query_components(reg, parent, [&, &plPos = position](vec3f& position)
                {
                    vec2f dpos = (position - plPos).xz();
                    vec2i delta = {int(roundf(dpos.x)), int(roundf(dpos.y))};
                    if (fabsf(dpos.x) > fabsf(dpos.y))
                        delta = vec2i(sign(delta.x), 0);
                    else
                        delta = vec2i(0, sign(delta.y));
                    for (int i = 0; i < shove_distance; ++i)
                    {
                        vec3f pplus = position + vec3f(delta.x, 0, delta.y);
                        if (check_collision_dir(reg, position, delta) || !check_floor(reg, pplus) || check_occupancy(reg, pplus))
                            break;
                        position = pplus;
                    }
                }, COMPID(vec3f, position));
            }
        }, eecs::ComponentId<int>(attribute.c_str()), COMPID(const vec3f, position));
    }, COMPID(const eecs::EntityId, parent), COMPID(const std::string, attribute), COMPID(const int, shove_distance));

    static Sound enemyHit = LoadSound("res/audio/sfx/hit.ogg");
    static Sound enemyDied = LoadSound("res/audio/sfx/enemy_perish.ogg");
    auto procAttack = [attrRoll](eecs::Registry& reg, const char* attrName, int attrVal, float attrMult, int dmg, const char* desc, int& hitpoints, int expDrop,
                         eecs::EntityId enemy, eecs::EntityId pl)
    {
        if (eecs::has_comp(reg, pl, COMPID(Tag, genius)))
        {
            attrMult *= 2.f;
            push_rolling_text(reg, "GeniusInj: wears out", WHITE);
            eecs::del_component(reg, pl, COMPID(Tag, genius));
        }
        if (!attrRoll(reg, attrName, attrVal, attrMult))
            return;
        push_rolling_text(reg, TextFormat("Damaged enemy for %d dmg %s", dmg, desc), GetColor(0x3e8948ff));
        hitpoints -= dmg;
        if (hitpoints <= 0)
        {
            PlaySound(enemyDied);
            add_exp(reg, pl, expDrop);
            eecs::del_entity(reg, enemy);
        }
        else
            PlaySound(enemyHit);
    };
    eecs::on_event(reg, FNV1(attack), [&](eecs::EntityId actEid, eecs::EntityId pl, eecs::EntityId parent, const std::string& attribute, float attr_dmgMult, eecs::EntityId action_sound)
    {
        const float attrMult = eecs::get_comp_or(reg, actEid, COMPID(float, attribute_modifier), 1.f);
        eecs::query_components(reg, parent, [&](int& hitpoints, int expDrop)
        {
            eecs::query_components(reg, pl, [&](int attr)
            {
                std::string attrName = eecs::get_comp_or(reg, eecs::find_entity(reg, attribute.c_str()), COMPID(std::string, name), attribute);
                eecs::query_components(reg, action_sound, [&](const Sound& sound)
                {
                    PlaySound(sound);
                }, COMPID(const Sound, sound));
                std::string dmgLine = std::string(TextFormat("(%s/%d)", attrName.c_str(), int(roundf(1.f / attr_dmgMult))));
                procAttack(reg, attrName.c_str(), attr, attrMult, attr * attr_dmgMult, dmgLine.c_str(), hitpoints, expDrop, parent, pl);
            }, eecs::ComponentId<int>(attribute.c_str()));
        }, COMPID(int, hitpoints), COMPID(const int, expDrop));
    }, COMPID(const eecs::EntityId, parent), COMPID(const std::string, attribute), COMPID(const float, attr_dmgMult), COMPID(const eecs::EntityId, action_sound));

    eecs::on_event(reg, FNV1(add_item), [&](eecs::EntityId actEid, eecs::EntityId pl, eecs::EntityId parent, const std::string& compName)
    {
        eecs::query_components(reg, pl, [&](int& item)
        {
            item++;
        }, eecs::ComponentId<int>(compName.c_str()));
        eecs::del_entity(reg, parent);
    }, COMPID(const eecs::EntityId, parent), COMPID(const std::string, compName));

    eecs::on_event(reg, FNV1(RegenX), [&](eecs::EntityId pl, eecs::EntityId, int& items_regenX, int& hitpoints, int attr_body)
    {
        if (hitpoints >= attr_body)
            return;
        const int numHeal = attr_body / 2;
        int prevHp = hitpoints;
        hitpoints = std::min(hitpoints + numHeal, attr_body);
        push_rolling_text(reg, TextFormat("RegenX: healed %d/%d hp", hitpoints - prevHp, numHeal), WHITE);
        items_regenX--;
    }, COMPID(int, items_regenX), COMPID(int, hitpoints), COMPID(const int, attr_body));

    constexpr int numTurns = 10;
    constexpr int potency = 25;

    auto procUse = [&](eecs::Registry& reg, eecs::EntityId pl, const char* turnsName, const char* name, const char* attrName, int& attr, int& items)
    {
        int curTurns = eecs::get_comp_or(reg, pl, eecs::ComponentId<int>(turnsName), 0);
        if (curTurns > 0)
            return;
        attr += potency;
        items--;
        eecs::set_component(reg, pl, eecs::ComponentId<int>(turnsName), numTurns);
        push_rolling_text(reg, TextFormat("%s: %s+%d for %dt", name, attrName, potency, numTurns), WHITE);
    };
    eecs::on_event(reg, FNV1(BearSerker), [&](eecs::EntityId pl, eecs::EntityId, int& items_bearserker, int& attr_strength)
    {
        procUse(reg, pl, "effect_bearSerkerTurns", "BearSerker", "STR", attr_strength, items_bearserker);
    }, COMPID(int, items_bearserker), COMPID(int, attr_strength));
    eecs::on_event(reg, FNV1(RefleXXX), [&](eecs::EntityId pl, eecs::EntityId, int& items_reflexxx, int& attr_agility)
    {
        procUse(reg, pl, "effect_reflexxxTurns", "RefleXXX", "AGI", attr_agility, items_reflexxx);
    }, COMPID(int, items_reflexxx), COMPID(int, attr_agility));
    eecs::on_event(reg, FNV1(MindDefoger), [&](eecs::EntityId pl, eecs::EntityId, int& items_mindDefoger, int& attr_mind)
    {
        procUse(reg, pl, "effect_minDefogerTurns", "MindDefog", "MIND", attr_mind, items_mindDefoger);
    }, COMPID(int, items_mindDefoger), COMPID(int, attr_mind));
    eecs::on_event(reg, FNV1(Bandito), [&](eecs::EntityId pl, eecs::EntityId, int& items_bandito, int& attr_strength, int& attr_agility, int& attr_mind)
    {
        int curTurns = eecs::get_comp_or(reg, pl, COMPID(int, effect_banditoTurns), 0);
        if (curTurns > 0)
            return;
        attr_strength += potency;
        attr_agility += potency;
        attr_mind += potency;
        items_bandito--;
        eecs::set_component(reg, pl, COMPID(int, effect_banditoTurns), numTurns);
        push_rolling_text(reg, TextFormat("Bandito: STR,AGI,MIND+%d for %dt", potency, numTurns), WHITE);
    }, COMPID(int, items_bandito), COMPID(int, attr_strength), COMPID(int, attr_agility), COMPID(int, attr_mind));
    eecs::on_event(reg, FNV1(GeniusInj), [&](eecs::EntityId pl, eecs::EntityId, int& items_genius)
    {
        if (eecs::has_comp(reg, pl, COMPID(Tag, genius)))
            return;
        items_genius--;
        push_rolling_text(reg, "GeniusInj: applied", WHITE);
        eecs::set_component(reg, pl, COMPID(Tag, genius), Tag{});
    }, COMPID(int, items_genius));

    auto procEffect = [&](eecs::Registry& reg, int& turns, int& attr, const char* name)
    {
        if (turns <= 0)
            return;
        turns--;
        if (turns == 0)
        {
            attr -= potency;
            push_rolling_text(reg, TextFormat("%s: wears out", name), WHITE);
        }
    };

    eecs::reg_system(reg, [&](eecs::EntityId, float& globalCd, float cooldownSplit)
    {
        float prevVal = globalCd;
        globalCd += GetFrameTime();
        if (prevVal < cooldownSplit && globalCd >= cooldownSplit)
        {
            eecs::query_entities(reg, [&](eecs::EntityId plEid, Tag player)
            {
                eecs::emit_event(reg, FNV1(next_turn), eecs::invalid_eid, plEid);
            }, COMPID(Tag, player));
        }
    }, COMPID(float, globalCd), COMPID(const float, cooldownSplit));

    eecs::on_event(reg, FNV1(next_turn), [&](eecs::EntityId, eecs::EntityId, float& globalCd, float cooldownSplit)
    {
        globalCd = cooldownSplit;
    }, COMPID(float, globalCd), COMPID(const float, cooldownSplit));

    eecs::on_event(reg, FNV1(next_turn), [&](eecs::EntityId, eecs::EntityId, int& effect_bearSerkerTurns, int& attr_strength)
    {
        procEffect(reg, effect_bearSerkerTurns, attr_strength, "BearSerker");
    }, COMPID(int, effect_bearSerkerTurns), COMPID(int, attr_strength));
    eecs::on_event(reg, FNV1(next_turn), [&](eecs::EntityId, eecs::EntityId, int& effect_reflexxxTurns, int& attr_agility)
    {
        procEffect(reg, effect_reflexxxTurns, attr_agility, "RefleXXX");
    }, COMPID(int, effect_reflexxxTurns), COMPID(int, attr_agility));
    eecs::on_event(reg, FNV1(next_turn), [&](eecs::EntityId, eecs::EntityId, int& effect_minDefogerTurns, int& attr_mind)
    {
        procEffect(reg, effect_minDefogerTurns, attr_mind, "Mind Defog");
    }, COMPID(int, effect_minDefogerTurns), COMPID(int, attr_mind));
    eecs::on_event(reg, FNV1(next_turn), [&](eecs::EntityId, eecs::EntityId, int& effect_banditoTurns, int& attr_strength, int& attr_agility, int& attr_mind)
    {
        if (effect_banditoTurns <= 0)
            return;
        effect_banditoTurns--;
        if (effect_banditoTurns == 0)
        {
            attr_strength -= potency;
            attr_agility -= potency;
            attr_mind -= potency;
            push_rolling_text(reg, "Bandito: wears out", WHITE);
        }
    }, COMPID(int, effect_banditoTurns), COMPID(int, attr_strength), COMPID(int, attr_agility), COMPID(int, attr_mind));

    eecs::on_event(reg, FNV1(open_remote), [&](eecs::EntityId eid, eecs::EntityId reply, Tag dialogue_active)
    {
        eecs::query_components(reg, reply, [&](const std::string& level_name, const std::vector<std::string>& door_tags)
        {
            eecs::Registry& levelReg = get_registry(level_name);
            eecs::query_entities(levelReg, [&](eecs::EntityId doorEid, const std::string& door_tag)
            {
                for (const std::string& dt : door_tags)
                    if (door_tag == dt)
                        eecs::emit_event(levelReg, FNV1(open), doorEid, doorEid);
            }, COMPID(const std::string, door_tag));
        }, COMPID(const std::string, level_name), COMPID(const std::vector<std::string>, door_tags));
    }, COMPID(Tag, dialogue_active));
}

