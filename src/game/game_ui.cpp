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

void draw_character(eecs::Registry& reg, float top, float width, float height, float scaleFactor)
{
    static Font charFont = LoadFontEx("res/textures/ui/8px-IBM_BIOS_8x8.ttf", 8, nullptr, 0);
    const vec2i cam_wh = get_cam_wh(reg);
    vec2f pos = {cam_wh.x + 50.f * scaleFactor, top};
    float step = 14.f * scaleFactor;
    float hzStep = 140 * scaleFactor;
    const float textSize = 8.f * scaleFactor;
    static Sound press = LoadSound("res/audio/sfx/press_01.ogg");
    auto drawAttr = [&](const char* str, vec2f pos, int& val, int& pointsToSpend)
    {
        DrawTextEx(charFont, TextFormat("%s: %d", str, val), toRLVec2(pos), textSize, 0, GetColor(0x3e8948ff));
        if (pointsToSpend > 0)
        {
            const Vector2 sz = MeasureTextEx(charFont, TextFormat("%s: %d", str, val), textSize, 0);
            float hzOffs = sz.x + 4.f * scaleFactor;
            vec2f upgPos = pos + vec2f(hzOffs, 0.f);
            auto drawPlus = [&](int v)
            {
                std::string text = "+" + std::to_string(v);
                Vector2 upgSz = MeasureTextEx(charFont, text.c_str(), textSize, 0);
                Vector2 mp = GetMousePosition();
                Rectangle upgRect = torect(upgPos.x, upgPos.y, upgSz.x, upgSz.y);
                if (is_vec_in_rect(mp, upgRect))
                {
                    DrawRectangleRec(upgRect, GetColor(0xfeae34ff));
                    DrawTextEx(charFont, text.c_str(), toRLVec2(upgPos), textSize, 0, GetColor(0x181425ff));
                    if (IsMouseButtonReleased(0))
                    {
                        PlaySound(press);
                        pointsToSpend -= v;
                        val += v;
                    }
                }
                else
                    DrawTextEx(charFont, text.c_str(), toRLVec2(upgPos), textSize, 0, GetColor(0xfeae34ff));
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

        DrawTextEx(charFont, TextFormat("LVL: %d", level), toRLVec2(pos), textSize, 0, GetColor(0x3e8948ff));
        DrawTextEx(charFont, TextFormat("XP: %d/%d", experience, nextLevelExp), toRLVec2(pos + vec2f(hzStep, 0)), textSize, 0, GetColor(0x3e8948ff)); pos.y += step;

        if (pointsToSpend > 0)
        {
            DrawTextEx(charFont, TextFormat("PTS: %d", pointsToSpend), toRLVec2(pos), textSize, 0, GetColor(0xfeae34ff)); pos.y += step;
        }

        drawAttr("STR", pos, attr_strength, pointsToSpend);
        drawAttr("AGI", pos + vec2f(hzStep, 0), attr_agility, pointsToSpend); pos.y += step;
        drawAttr("MIND", pos, attr_mind, pointsToSpend);
        drawAttr("BODY", pos + vec2f(hzStep, 0), attr_body, pointsToSpend); pos.y += step;

        DrawTextEx(charFont, TextFormat("HP: %d/%d", hitpoints, attr_body), toRLVec2(pos), textSize, 0, GetColor(0x3e8948ff)); pos.y += step;

    }, COMPID(int, attr_strength), COMPID(int, attr_agility), COMPID(int, attr_mind), COMPID(int, attr_body), COMPID(const int, hitpoints),
        COMPID(const int, level), COMPID(const int, experience), COMPID(int, pointsToSpend));
}

void draw_ui(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    static Font headerFont = LoadFontEx("res/textures/ui/16px-IBM_VGA_8x16.ttf", 16, nullptr, 0);
    static Font logFont = LoadFontEx("res/textures/ui/8px-IBM_BIOS_8x8.ttf", 8, nullptr, 0);
    static Texture2D border = LoadTexture("res/textures/ui/border.png");
    static Texture2D horzborder = LoadTexture("res/textures/ui/horz_border2.png");
    const float headerSize = 16.f * scaleFactor;

    float charEnd = (40 + 14 * 5) * scaleFactor;

    const float uiPxScale = 4.f * scaleFactor;
    const float borderHt = uiPxScale * 8.f;
    const float pad = 4.f * scaleFactor;
    const float actEnd = height - 180 * scaleFactor;

    eecs::query_entities(reg, [&](eecs::EntityId, int cam_resWidth, float cam_resMult)
    {
        const float leftSide = cam_resWidth * cam_resMult * scaleFactor - cam_resMult * scaleFactor;
        const float leftSideTrue = leftSide + cam_resMult * scaleFactor;
        draw_tiled_tex(horzborder, torect(0, 0, width, 12), torect(leftSideTrue, charEnd, width, 12 * scaleFactor), uiPxScale, WHITE);
        draw_tiled_tex(horzborder, torect(0, 0, width, 12), torect(leftSideTrue, actEnd, width, 12 * scaleFactor), uiPxScale, WHITE);

        draw_centered_font_with_shadow(headerFont, "CHARACTER", torect(leftSide + 12.f * uiPxScale, 16.f * scaleFactor, width - (leftSide + 12.f * uiPxScale), headerSize), headerSize, 3, GetColor(0x3e8948ff));
        draw_centered_font_with_shadow(headerFont, "ACTIONS", torect(leftSide + 12.f * uiPxScale, charEnd + borderHt, width - (leftSide + 12.f * uiPxScale), headerSize), headerSize, 3, GetColor(0x3e8948ff));
        draw_centered_font_with_shadow(headerFont, "SYSLOG", torect(leftSide + 12.f * uiPxScale, actEnd + borderHt, width - (leftSide + 12.f * uiPxScale), headerSize), headerSize, 3, GetColor(0x3e8948ff));

        draw_tiled_tex(border, torect(0, 0, 12, height), torect(leftSide, 0.f, 12 * scaleFactor, height), uiPxScale, WHITE);
    }, COMPID(const int, cam_resWidth), COMPID(const float, cam_resMult));

    draw_character(reg, 40.f * scaleFactor, width, height, scaleFactor);
    //DrawFPS(20, height - 40);
    draw_interactables(reg, charEnd + borderHt + headerSize + pad, width, height, scaleFactor);

    const vec2i cam_wh = get_cam_wh(reg);
    vec2f pos = {cam_wh.x + 50.f * scaleFactor, actEnd + borderHt + headerSize + pad};
    const float spacing = 12.f * scaleFactor;
    eecs::query_entities(reg, [&](eecs::EntityId, std::vector<ColoredText>& rollingText)
    {
        int numfit = (height - pos.y) / spacing;
        while (numfit < rollingText.size())
            rollingText.erase(rollingText.begin());
        for (const ColoredText& text : rollingText)
        {
            DrawTextEx(logFont, text.first.c_str(), toRLVec2(pos), 8.f * scaleFactor, 0, text.second);
            pos.y += spacing;
        }
    }, COMPID(const std::vector<ColoredText>, rollingText));
}

void push_rolling_text(eecs::Registry& reg, const char* text, Color col)
{
    eecs::query_component(reg, eecs::find_entity(reg, "rolling_text"), [&](std::vector<ColoredText>& rollingText)
    {
        rollingText.push_back(std::make_pair(std::string(text), col));
    }, COMPID(std::vector<ColoredText>, rollingText));
}

