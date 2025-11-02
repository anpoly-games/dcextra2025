#include <eecs.h>
#include <raylib.h>
#include <random>

#include "../math.h"
#include "../tags.h"
#include "../level.h"
#include "../game.h"
#include "triggers.h"
#include "game_ui.h"


void register_triggers(eecs::Registry& reg)
{
    eecs::reg_system(reg, [&](eecs::EntityId playerEid, vec3i& prevPosition, const vec3f& position)
    {
        vec3i curPosition = pos_to_grid3d(position);
        if (curPosition == prevPosition)
            return;
        prevPosition = curPosition;
        eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, const vec2i& trigger_volume)
        {
            vec3i triggerPos = pos_to_grid3d(position);
            vec2i volExt = vec2i((trigger_volume.x - 1) / 2, (trigger_volume.y - 1) / 2);
            if (triggerPos.y == curPosition.y &&
                curPosition.x >= triggerPos.x - volExt.x && curPosition.x <= triggerPos.x + volExt.x &&
                curPosition.z >= triggerPos.z - volExt.y && curPosition.z <= triggerPos.z + volExt.y)
            {
                eecs::emit_event(reg, FNV1(enterTrigger), eid, playerEid);
            }
        }, COMPID(const vec3f, position), COMPID(const vec2i, trigger_volume));
    }, COMPID(vec3i, prevPosition), COMPID(const vec3f, position));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId eid, eecs::EntityId plEid, const std::vector<std::string>& dialogue_text)
    {
        if (!eecs::has_comp(reg, eid, COMPID(Tag, dialogue_active)) && eecs::has_comp(reg, plEid, COMPID(Tag, player)))
            eecs::set_component(reg, eid, COMPID(Tag, dialogue_active), Tag{});
    }, COMPID(const std::vector<std::string>, dialogue_text));

    eecs::on_event(reg, FNV1(kill_dialogue), [&](eecs::EntityId eid, eecs::EntityId, Tag dialogue_active)
    {
        eecs::del_entity(reg, eid);
    }, COMPID(const Tag, dialogue_active));
    eecs::on_event(reg, FNV1(kill_all_dialogues), [&](eecs::EntityId eid, eecs::EntityId, Tag dialogue_active)
    {
        eecs::query_entities(reg, [&](eecs::EntityId deid, const std::vector<std::string>& dialogue_text)
        {
            eecs::del_entity(reg, deid);
        }, COMPID(const std::vector<std::string>, dialogue_text));
    }, COMPID(const Tag, dialogue_active));
    eecs::on_event(reg, FNV1(close_dialogue), [&](eecs::EntityId eid, eecs::EntityId, Tag dialogue_active)
    {
        eecs::del_component(reg, eid, COMPID(Tag, dialogue_active));
    }, COMPID(const Tag, dialogue_active));

    eecs::on_event(reg, FNV1(join_team_1), [&](eecs::EntityId, eecs::EntityId, Tag dialogue_active)
    {
        eecs::query_entities(reg, [&](eecs::EntityId plEid, Tag player, int& team)
        {
            team = 1;
        }, COMPID(Tag, player), COMPID(int, team));
    }, COMPID(const Tag, dialogue_active));

    eecs::on_event(reg, FNV1(join_team_2), [&](eecs::EntityId, eecs::EntityId, Tag dialogue_active)
    {
        eecs::query_entities(reg, [&](eecs::EntityId plEid, Tag player, int& team)
        {
            team = 2;
        }, COMPID(Tag, player), COMPID(int, team));
    }, COMPID(const Tag, dialogue_active));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId, eecs::EntityId plEid, const std::string& level_switchTo)
    {
        if (!eecs::has_comp(reg, plEid, COMPID(Tag, player)))
            return;
        printf("switching to level %s\n", level_switchTo.c_str());
        //load_level(reg, level_switchTo.c_str());
        //change_level(reg, level_switchTo.c_str());
        eecs::create_or_find_entity_wrap(reg, "Switch_Level").set(COMPID(std::string, nextLevel), level_switchTo);
    }, COMPID(const std::string, level_switchTo));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId eid, eecs::EntityId playerId, const vec2f& displacement, Tag teleporter)
    {
        eecs::query_components(reg, playerId, [&](vec3f& position, Tag player)
        {
            position.x += displacement.x;
            position.z += displacement.y; // displacement is in the x-z plane
        }, COMPID(vec3f, position), COMPID(Tag, player));
    }, COMPID(vec2f, displacement), COMPID(Tag, teleporter));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId eid, eecs::EntityId playerId, const float rel_displacement, Tag relative_teleporter)
    {
        eecs::query_components(reg, playerId, [&](vec3f& position, const vec3f& direction, Tag player)
        {
            position = position + rel_displacement * direction;
        }, COMPID(vec3f, position), COMPID(const vec3f, direction), COMPID(Tag, player));
    }, COMPID(float, rel_displacement), COMPID(Tag, relative_teleporter));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId eid, eecs::EntityId playerId, const vec3f& ori_displacement, float rotation)
    {
        eecs::query_components(reg, playerId, [&](vec3f& position)
        {
            if (eecs::has_comp(reg, playerId, COMPID(Tag, player)))
            {
                eecs::query_components(reg, eid, [&](eecs::EntityId tele_sound)
                {
                    eecs::query_components(reg, tele_sound, [&](const Sound& sound)
                    {
                        PlaySound(sound);
                    }, COMPID(const Sound, sound));
                }, COMPID(eecs::EntityId, tele_sound));
            }
            position = position + vec3f(0, ori_displacement.y, 0) +
                vec3f(sinf(rotation * DEG2RAD), 0, cosf(rotation * DEG2RAD)) * ori_displacement.z +
                vec3f(cosf(rotation * DEG2RAD), 0, -sinf(rotation * DEG2RAD)) * ori_displacement.x;
        }, COMPID(vec3f, position));
    }, COMPID(const vec3f, ori_displacement), COMPID(const float, rotation));

    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId eid, eecs::EntityId playerId, const int turns, Tag spinner)
    {
        eecs::query_components(reg, playerId, [&](vec3f& direction, Tag player)
        {
            if (turns==2)
            {
                direction.x = -direction.x;
                direction.z = -direction.z;
            }
            else
            {
                direction = vec3{-turns * direction.z, direction.y, turns * direction.x};
            }
        }, COMPID(vec3f, direction), COMPID(Tag, player));
    }, COMPID(const int, turns), COMPID(Tag, spinner));

    eecs::on_event(reg, FNV1(win_game), [&](eecs::EntityId, eecs::EntityId, const std::string& triggers)
    {
        set_game_state(E_WIN);
    }, COMPID(const std::string, triggers));

    static Sound playerHit = LoadSound("res/audio/sfx/hit_07.ogg");
    eecs::on_event(reg, FNV1(enterTrigger), [&](eecs::EntityId, eecs::EntityId plEid, int area_damage, Tag active)
    {
        eecs::query_components(reg, plEid, [&](int& hitpoints)
        {
            PlaySound(playerHit);
            push_rolling_text(reg, TextFormat("Received %d dmg", area_damage), GetColor(0xff0044ff));
            hitpoints = std::max(0, hitpoints - area_damage);
        }, COMPID(int, hitpoints));
    }, COMPID(const int, area_damage), COMPID(Tag, active));

    eecs::reg_enter(reg, [&](eecs::EntityId eid, const vec3f& particles_volume, Tag active)
    {
        eecs::set_component(reg, eid, eecs::ComponentId<std::vector<std::pair<vec3f, vec3f>>>("particles"), std::vector<std::pair<vec3f, vec3f>>());
    }, COMPID(const vec3f, particles_volume), COMPID(Tag, active));

    eecs::reg_exit(reg, [&](eecs::EntityId eid, const vec3f& particles_volume, Tag active)
    {
        eecs::del_component(reg, eid, eecs::ComponentId<std::vector<std::pair<vec3f, vec3f>>>("particles"));
    }, COMPID(const vec3f, particles_volume), COMPID(Tag, active));

    eecs::reg_system(reg, [&](eecs::EntityId, const vec3f& position, const vec3f& particles_volume, int particles_num, float particles_speed, std::vector<std::pair<vec3f, vec3f>>& particles)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        while (particles.size() < particles_num)
        {
            vec3f p = position + 0.5f * vec3f(dis(gen), dis(gen), dis(gen)) * particles_volume + vec3f(0, 0.5f, 0);
            vec3f vel = particles_speed * vec3f(dis(gen), dis(gen), dis(gen)).normalized();
            particles.push_back(std::make_pair(p, vel));
        }
        const vec3f lb = position - 0.5f * particles_volume + vec3f(0, 0.5f, 0);
        const vec3f hb = position + 0.5f * particles_volume + vec3f(0, 0.5f, 0);
        for (int i = int(particles.size()) - 1; i >= 0; --i)
        {
            vec3f& pos = particles[i].first;
            vec3f& vel = particles[i].second;
            pos += GetFrameTime() * vel;
            if (pos.x < lb.x || pos.y < lb.y || pos.z < lb.z || pos.x > hb.x || pos.y > hb.y || pos.z > hb.z)
                particles.erase(particles.begin() + i);
        }
    }, COMPID(const vec3f, position), COMPID(const vec3f, particles_volume), COMPID(const int, particles_num), COMPID(const float, particles_speed), eecs::ComponentId<const std::vector<std::pair<vec3f, vec3f>>>("particles"));

    eecs::reg_system(reg, [&](eecs::EntityId, const std::vector<std::pair<vec3f, vec3f>>& particles, const vec4f& particles_color)
    {
        Color col = ColorFromNormalized(toRLVec4(particles_color));
        const float cubeSz = 0.05f;
        for (auto& particle : particles)
            DrawCube(toRLVec3(particle.first), cubeSz, cubeSz, cubeSz, col);
    }, eecs::ComponentId<const std::vector<std::pair<vec3f, vec3f>>>("particles"), COMPID(const vec4f, particles_color));

    eecs::on_event(reg, FNV1(next_turn), [&](eecs::EntityId eid, eecs::EntityId, int& turnsTillActive, int turnCycle, int turnsActive)
    {
        turnsTillActive--;
        if (turnsTillActive == 0)
        {
            eecs::set_component(reg, eid, COMPID(Tag, active), Tag{});
        }
        if (turnsTillActive <= -turnsActive)
        {
            turnsTillActive = turnCycle;
            eecs::del_component(reg, eid, COMPID(Tag, active));
        }
    }, COMPID(int, turnsTillActive), COMPID(const int, turnCycle), COMPID(const int, turnsActive));

    eecs::on_event(reg, FNV1(next_turn), [&](eecs::EntityId eid, eecs::EntityId, const vec3f& position, int area_damage, Tag active, const vec2i& trigger_volume)
    {
        vec3i triggerPos = pos_to_grid3d(position);
        eecs::query_entities(reg, [&](eecs::EntityId playerEid, const vec3f& position, Tag player)
        {
            vec3i curPosition = pos_to_grid3d(position);
            vec2i volExt = vec2i((trigger_volume.x - 1) / 2, (trigger_volume.y - 1) / 2);
            if (triggerPos.y == curPosition.y &&
                curPosition.x >= triggerPos.x - volExt.x && curPosition.x <= triggerPos.x + volExt.x &&
                curPosition.z >= triggerPos.z - volExt.y && curPosition.z <= triggerPos.z + volExt.y)
            {
                eecs::emit_event(reg, FNV1(enterTrigger), eid, playerEid);
            }
        }, COMPID(const vec3f, position), COMPID(Tag, player));
    }, COMPID(const vec3f, position), COMPID(const int, area_damage), COMPID(Tag, active), COMPID(const vec2i, trigger_volume));

    eecs::reg_system(reg, [&](eecs::EntityId, int& prevHitpoints, float& timeSinceHit, int hitpoints)
    {
        timeSinceHit += GetFrameTime();
        if (prevHitpoints == hitpoints)
            return;
        if (hitpoints < prevHitpoints)
            timeSinceHit = 0.f;
        prevHitpoints = hitpoints;
    }, COMPID(int, prevHitpoints), COMPID(float, timeSinceHit), COMPID(int, hitpoints));


    eecs::reg_system(reg, [&](eecs::EntityId, vec3f& modelOffs, float timeSinceHit)
    {
        if (timeSinceHit > 0.2f)
            return;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        const float mult = 0.2f - timeSinceHit;
        modelOffs = mult * vec3f(dis(gen), dis(gen), dis(gen));
    }, COMPID(vec3f, modelOffs), COMPID(const float, timeSinceHit));

    eecs::reg_system(reg, [&](eecs::EntityId, int hitpoints, Tag player)
    {
        if (hitpoints <= 0)
            set_game_state(E_LOSE);
    }, COMPID(const int, hitpoints), COMPID(const Tag, player));
}

