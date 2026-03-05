#include <raylib.h>
#include <eecs.h>

#include "dcengine/game.h"
#include "dcengine/ui.h"
#include "dcengine/textures.h"
#include "dcengine/primitives.h"
#include "dcengine/player.h"
#include "dcengine/audio.h"
#include "dcengine/renderer.h"
#include "dcengine/cam.h"
#include "dcengine/level.h"
#include "interactables.h"
#include "triggers.h"
#include "debug.h"
#include "dcengine/prefabs.h"
#include "spawn.h"
#include "ai.h"

void register_systems(eecs::Registry& reg)
{
  register_textures(reg);
  register_primitives(reg);
  register_player(reg);
  register_audio(reg);
  register_renderer(reg);
  register_cam(reg);
  register_level(reg);
  register_ui(reg);
  register_spawn(reg);
  register_triggers(reg);
  register_interactables(reg);
  register_ai(reg);
  if (eecs::find_entity(reg, "DebugMarker")!=eecs::invalid_eid)
    register_debug(reg);
}

eecs::EntityId init_new_world(eecs::Registry& reg)
{
    eecs::EntityId eid = create_cam(reg);
    eecs::EntityId playerEid = create_player(reg);
    return playerEid;
}

void restart_world(eecs::Registry& reg)
{
    float width = -1.f, height = -1.f, scaleFactor = 0.f;
    eecs::query_entities(reg, [&](eecs::EntityId, float window_width, float window_height, float window_scaleFactor)
    {
        width = window_width;
        height = window_height;
        scaleFactor = window_scaleFactor;
    }, COMPID(const float, window_width), COMPID(const float, window_height), COMPID(const float, window_scaleFactor));

    bool debugMode = eecs::find_entity(reg, "DebugMarker")!=eecs::invalid_eid;

    eecs::del_all_entities(reg);
    eecs::del_all_systems(reg);
    if (debugMode)
    {
        eecs::EntityId eid = eecs::create_entity(reg, "DebugMarker"); // existence of this entity means we are in debug mode
    }
    register_systems(reg);
    init_new_world(reg);
    if (scaleFactor > 0.f)
        create_ui_helper(reg, width, height, scaleFactor);
}
