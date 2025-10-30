#include <eecs.h>
#include <raylib.h>

#include "game_ui.h"
#include "advancement.h"

void add_exp(eecs::Registry& reg, eecs::EntityId plEid, int amount)
{
    static Sound levelUp = LoadSound("res/audio/sfx/upbeat_01.ogg");
    eecs::query_components(reg, plEid, [&](int& experience, int& level, int& pointsToSpend)
    {
        experience += amount;
        while (experience >= get_next_level_exp(level))
        {
            experience -= get_next_level_exp(level);
            pointsToSpend += get_points_per_level(); // TODO: component maybe? MIND?
            PlaySound(levelUp);
            level++;
            eecs::query_component(reg, eecs::find_entity(reg, "rolling_text"), [&](std::vector<ColoredText>& rollingText)
            {
                rollingText.push_back(std::make_pair(std::string(TextFormat("You've advanced to level %d!", level)), GetColor(0xfee761ff)));
                rollingText.push_back(std::make_pair(std::string(TextFormat("You have %d points to spend!", pointsToSpend)), GetColor(0xfee761ff)));
            }, COMPID(std::vector<ColoredText>, rollingText));
        }
    }, COMPID(int, experience), COMPID(int, level), COMPID(int, pointsToSpend));
}

