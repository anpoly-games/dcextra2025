#pragma once

inline int get_next_level_exp(int level)
{
    return 100;
}

inline int get_points_per_level()
{
    return 10;
}

void add_exp(eecs::Registry& reg, eecs::EntityId plEid, int amount);

