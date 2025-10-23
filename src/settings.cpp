#include <flecs.h>
#include <string>

#include "settings.h"

struct settings
{
  settings(flecs::world& ecs)
  {
    ecs.module<settings>();

    ecs.component<OptInstantStep>().member<bool>("val");
    ecs.component<OptMusicVolume>().member<float>("val");
    ecs.component<OptSfxVolume>().member<float>("val");
  }
};

void register_settings(flecs::world& ecs)
{
  ecs.import<settings>();
}

void create_settings(flecs::world& ecs)
{
  ecs.entity("settings")
    .set(OptInstantStep{false})
    .set(OptMusicVolume{0.5f})
    .set(OptSfxVolume{0.5f});
}

std::string save_settings(flecs::world& ecs)
{
  return std::string(ecs.lookup("settings").to_json());
}

void load_settings(flecs::world& ecs, const std::string& json)
{
  ecs.entity("settings").from_json(json.c_str());
}

