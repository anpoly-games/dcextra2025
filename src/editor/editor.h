#pragma once

void register_editor(eecs::Registry& reg, flecs::world& ecs);

void save_level(flecs::world& ecs, const char* filename);

enum EntTypeList
{
  E_FLOORS = 0,
  E_WALLS,
  E_DOORS,
  E_COLUMNS,
  E_CEILINGS,
  E_ENTITIES,
  E_LOGIC,
  E_ETL_NUM
};

struct SelectedType
{
  EntTypeList type;
  std::string prefab;
};

struct SelectedEntity {};

