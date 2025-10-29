#include <eecs.h>
#include <raylib.h>

#include "../math.h"
#include "../cam.h"
#include "../ui.h"
#include "game_ui.h"
#include "interactables.h"
#include "advancement.h"

bool is_ui_blocks_input()
{
    return false;
}

void draw_character(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    static Font charFont = LoadFont("res/textures/ui/16px-IBM_VGA_8x16.ttf");
    const vec2i cam_wh = get_cam_wh(reg);
    vec2f pos = {cam_wh.x + 50.f * scaleFactor, 20.f * scaleFactor};
    float step = 22.f * scaleFactor;
    float hzStep = 140 * scaleFactor;
    const float textSize = 16.f * scaleFactor;
    auto drawAttr = [&](const char* str, vec2f pos, int& val, int& pointsToSpend)
    {
        DrawTextEx(charFont, TextFormat("%s: %d", str, val), toRLVec2(pos), textSize, 3, GetColor(0x3e8948ff));
        if (pointsToSpend > 0)
        {
            const Vector2 sz = MeasureTextEx(charFont, TextFormat("%s: %d", str, val), textSize, 3);
            float hzOffs = sz.x + 4.f * scaleFactor;
            vec2f upgPos = pos + vec2f(hzOffs, 0.f);
            auto drawPlus = [&](int v)
            {
                std::string text = "+" + std::to_string(v);
                Vector2 upgSz = MeasureTextEx(charFont, text.c_str(), textSize, 3);
                Vector2 mp = GetMousePosition();
                Rectangle upgRect = torect(upgPos.x, upgPos.y, upgSz.x, upgSz.y);
                if (is_vec_in_rect(mp, upgRect))
                {
                    DrawRectangleRec(upgRect, GetColor(0xfeae34ff));
                    DrawTextEx(charFont, text.c_str(), toRLVec2(upgPos), textSize, 3, GetColor(0x181425ff));
                    if (IsMouseButtonReleased(0))
                    {
                        pointsToSpend -= v;
                        val += v;
                    }
                }
                else
                    DrawTextEx(charFont, text.c_str(), toRLVec2(upgPos), textSize, 3, GetColor(0xfeae34ff));
                upgPos.x += upgSz.x + 4.f * scaleFactor;
            };
            drawPlus(1);
            if (pointsToSpend >= 5)
                drawPlus(5);
        }
    };
    eecs::query_entities(reg, [&](eecs::EntityId, int& attr_strength, int& attr_agility, int& attr_mind, int& attr_body, int hitpoints, int level, int experience, int& pointsToSpend)
    {
        int nextLevelExp = get_next_level_exp(level);

        DrawTextEx(charFont, "CHARACTER", toRLVec2(pos), textSize, 3, GetColor(0x3e8948ff)); pos.y += step;
        DrawTextEx(charFont, TextFormat("LVL: %d", level), toRLVec2(pos), textSize, 3, GetColor(0x3e8948ff));
        DrawTextEx(charFont, TextFormat("XP: %d/%d", experience, nextLevelExp), toRLVec2(pos + vec2f(hzStep, 0)), textSize, 3, GetColor(0x3e8948ff)); pos.y += step;

        if (pointsToSpend > 0)
        {
            DrawTextEx(charFont, TextFormat("PTS: %d", pointsToSpend), toRLVec2(pos), textSize, 3, GetColor(0xfeae34ff)); pos.y += step;
        }

        drawAttr("STR", pos, attr_strength, pointsToSpend);
        drawAttr("AGI", pos + vec2f(hzStep, 0), attr_agility, pointsToSpend); pos.y += step;
        drawAttr("MIND", pos, attr_mind, pointsToSpend);
        drawAttr("BODY", pos + vec2f(hzStep, 0), attr_body, pointsToSpend); pos.y += step;

        DrawTextEx(charFont, TextFormat("HP: %d/%d", hitpoints, attr_body), toRLVec2(pos), textSize, 3, GetColor(0x3e8948ff)); pos.y += step;

    }, COMPID(int, attr_strength), COMPID(int, attr_agility), COMPID(int, attr_mind), COMPID(int, attr_body), COMPID(const int, hitpoints),
        COMPID(const int, level), COMPID(const int, experience), COMPID(int, pointsToSpend));
}

void draw_ui(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    static Texture2D border = LoadTexture("res/textures/ui/border.png");

    eecs::query_entities(reg, [&](eecs::EntityId, int cam_resWidth, float cam_resMult)
    {
        const float leftSide = cam_resWidth * cam_resMult * scaleFactor - cam_resMult * scaleFactor;
        draw_tiled_tex(border, torect(0, 0, 12, height), torect(leftSide, 0.f, 12 * scaleFactor, height), scaleFactor * cam_resMult, WHITE);
    }, COMPID(const int, cam_resWidth), COMPID(const float, cam_resMult));

    draw_character(reg, width, height, scaleFactor);
    //DrawFPS(20, height - 40);
    draw_interactables(reg, width, height, scaleFactor);

    const vec2i cam_wh = get_cam_wh(reg);
    vec2f pos = {cam_wh.x + 50.f * scaleFactor, height - 100 * scaleFactor};
    const float spacing = 20.f * scaleFactor;
    eecs::query_entities(reg, [&](eecs::EntityId, std::vector<ColoredText>& rollingText)
    {
        int numfit = (height - pos.y) / spacing;
        while (numfit <= rollingText.size())
            rollingText.erase(rollingText.begin());
        for (const ColoredText& text : rollingText)
        {
            DrawTextEx(GetFontDefault(), text.first.c_str(), toRLVec2(pos), 12.f * scaleFactor, 3, text.second);
            pos.y += spacing;
        }
    }, COMPID(const std::vector<ColoredText>, rollingText));
}

