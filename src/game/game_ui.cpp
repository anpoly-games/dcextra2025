#include <eecs.h>
#include <raylib.h>

#include "../math.h"
#include "../cam.h"
#include "../ui.h"
#include "../tags.h"
#include "../game.h"
#include "game_ui.h"
#include "interactables.h"
#include "advancement.h"

static GameState gameState = E_MAIN_MENU;
static float timeFromChange = 0.f;

void set_game_state(GameState gs)
{
    if (gs != gameState)
        timeFromChange = 0.f;
    gameState = gs;
}


bool is_ui_blocks_input(eecs::Registry& reg)
{
    bool res = gameState != E_GAME;
    eecs::query_entities(reg, [&](eecs::EntityId, Tag dialogue_active)
    {
        res = true;
    }, COMPID(Tag, dialogue_active));
    return res;
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

        DrawTextEx(charFont, TextFormat("HP: %d/%d", hitpoints, attr_body), toRLVec2(pos), textSize, 0, GetColor(0x3e8948ff)); pos.y += step;
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
    }, COMPID(int, attr_strength), COMPID(int, attr_agility), COMPID(int, attr_mind), COMPID(int, attr_body), COMPID(const int, hitpoints),
        COMPID(const int, level), COMPID(const int, experience), COMPID(int, pointsToSpend));
}

void draw_items(eecs::Registry& reg, float top, float scrwidth, float height, float scaleFactor)
{
    static Font itmFont = LoadFontEx("res/textures/ui/8px-IBM_BIOS_8x8.ttf", 8, nullptr, 0);
    static NineRect nrect = create_9rect(LoadImage("res/textures/ui/button_rect.png"), 2);
    static Texture2D regenX = LoadTexture("res/textures/items/regenX.png");
    static Texture2D bearSerker = LoadTexture("res/textures/items/BearSerker.png");
    static Texture2D reflexxx = LoadTexture("res/textures/items/reflexxx.png");
    static Texture2D mindDefoger = LoadTexture("res/textures/items/mindDefoger.png");
    static Texture2D genius = LoadTexture("res/textures/items/genius.png");
    static Texture2D bandito = LoadTexture("res/textures/items/bandito.png");
    const vec2i cam_wh = get_cam_wh(reg);
    vec2f pos = {cam_wh.x + 50.f * scaleFactor, top};
    const float pad = 4.f * scaleFactor;
    eecs::query_entities(reg, [&](eecs::EntityId plEid,
                int& items_regenX, int& items_bearserker, int& items_reflexxx,
                int& items_mindDefoger, int& items_genius, int& items_bandito)
    {
        const float width = (scrwidth - pos.x - 4.f * 4.f * scaleFactor - pad) * 0.5f;
        const float step = 16.f * scaleFactor;
        auto drawItem = [&](const char* name, const Texture2D& icon, int& count, const vec2f& pos)
        {
            draw_button_9rect(nrect, Rectangle(pos.x, pos.y, width, step), itmFont, "", 8.f, 0, scaleFactor, ColorFromHSV(0, 0, 0.7f),
            [&]()
            {
                if (count > 0)
                {
                    if (is_ui_blocks_input(reg))
                        return;
                    eecs::emit_event(reg, fnv1StrHash(name), plEid, plEid);
                    eecs::emit_event(reg, FNV1(next_turn), eecs::invalid_eid, plEid);
                }
            });
            float vpos = pos.y + (step - icon.height * scaleFactor) * 0.5f;
            float hpos = pos.x + pad;
            DrawTextureEx(icon, {hpos, vpos}, 0.f, scaleFactor, WHITE);
            std::string finalText = std::string(name) + ": " + std::to_string(count);
            draw_centered_font_with_shadow(itmFont, finalText.c_str(), torect(hpos + icon.width * scaleFactor + pad, pos.y, width, step), 8.f * scaleFactor, 0, WHITE, EFC_VCENTER);
        };
        drawItem("RegenX", regenX, items_regenX, pos);
        drawItem("BearSerker", bearSerker, items_bearserker, pos + vec2f(width + pad, 0.f)); pos.y += step + pad;
        drawItem("RefleXXX", reflexxx, items_reflexxx, pos);
        drawItem("MindDefoger", mindDefoger, items_mindDefoger, pos + vec2f(width + pad, 0.f)); pos.y += step + pad;
        drawItem("GeniusInj", genius, items_genius, pos);
        drawItem("Bandito", bandito, items_bandito, pos + vec2f(width + pad, 0.f)); pos.y += step + 4.f * scaleFactor;
    }, COMPID(int, items_regenX), COMPID(int, items_bearserker), COMPID(int, items_reflexxx),
       COMPID(int, items_mindDefoger), COMPID(int, items_genius), COMPID(int, items_bandito));
}

void draw_dialogue(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    //static Texture2D border = LoadTexture("res/textures/ui/border.png");
    //static Texture2D horzborder = LoadTexture("res/textures/ui/horz_border2.png");
    static Font titleFont = LoadFontEx("res/textures/ui/16px-IBM_VGA_8x16.ttf", 16, nullptr, 0);
    static Font textFont = LoadFontEx("res/textures/ui/16px-IBM_VGA_8x16.ttf", 16, nullptr, 0);//LoadFontEx("res/textures/ui/8px-IBM_BIOS_8x8.ttf", 8, nullptr, 0);
    static NineRect nrect = create_9rect(LoadImage("res/textures/ui/button_rect.png"), 2);

    static Texture2D tlTex = LoadTexture("res/textures/ui/topleft.png");
    static Texture2D trTex = LoadTexture("res/textures/ui/topright.png");
    static Texture2D blTex = LoadTexture("res/textures/ui/botleft.png");
    static Texture2D brTex = LoadTexture("res/textures/ui/botright.png");
    static Texture2D hzTex = LoadTexture("res/textures/ui/horz.png");
    static Texture2D vtTex = LoadTexture("res/textures/ui/vert.png");
    const vec2i cam_wh = get_cam_wh(reg);
    const vec2f sz = {cam_wh.x * 0.8f, cam_wh.y * 0.6f};

    const float uiPxScale = 2.f * scaleFactor;
    const float pad = 4.f * scaleFactor;
    const float titleSz = 16.f * scaleFactor;
    const float fontSz = 16.f * scaleFactor;
    vec2f pos = 0.5f * vec2f(cam_wh.x - sz.x, cam_wh.y - sz.y);

    eecs::query_entities(reg, [&](eecs::EntityId eid, const std::string& dialogue_title,
                const std::vector<std::string>& dialogue_text, const std::vector<eecs::EntityId>& dialogue_replies, Tag dialogue_active)
    {
        eecs::query_entities(reg, [&](eecs::EntityId, int cam_resWidth, float cam_resMult)
        {
            const float texSz = uiPxScale * 12;
            DrawRectangleRec(torect(pos.x, pos.y, sz.x, sz.y), BLACK);
            DrawTexturePro(tlTex, torect(0, 0, 12, 12), torect(pos.x, pos.y, texSz, texSz), Vector2{0, 0}, 0, WHITE);
            DrawTexturePro(trTex, torect(0, 0, 12, 12), torect(pos.x + sz.x - texSz, pos.y, texSz, texSz), Vector2{0, 0}, 0, WHITE);
            DrawTexturePro(blTex, torect(0, 0, 12, 12), torect(pos.x, pos.y + sz.y - texSz, texSz, texSz), Vector2{0, 0}, 0, WHITE);
            DrawTexturePro(brTex, torect(0, 0, 12, 12), torect(pos.x + sz.x - texSz, pos.y + sz.y - texSz, texSz, texSz), Vector2{0, 0}, 0, WHITE);

            draw_tiled_tex(hzTex, torect(0, 0, (sz.x - texSz*2) / uiPxScale, 12), torect(pos.x + texSz, pos.y, 0, 0), uiPxScale, WHITE);
            draw_tiled_tex(hzTex, torect(0, 0, (sz.x - texSz*2) / uiPxScale, 12), torect(pos.x + texSz, pos.y + sz.y - texSz, 0, 0), uiPxScale, WHITE);
            draw_tiled_tex(vtTex, torect(0, 0, 12, (sz.y - texSz*2) / uiPxScale), torect(pos.x, pos.y + texSz, 0, 0), uiPxScale, WHITE);
            draw_tiled_tex(vtTex, torect(0, 0, 12, (sz.y - texSz*2) / uiPxScale), torect(pos.x + sz.x - texSz, pos.y + texSz, 0, 0), uiPxScale, WHITE);
            vec2f textPos = pos + vec2f{texSz + pad, texSz + pad};
            for (const std::string& line : dialogue_text)
            {
                std::string finalText = dialogue_title + "> " + line;
                DrawTextEx(textFont, finalText.c_str(), toRLVec2(textPos), fontSz, 0, GetColor(0x63c74dff));
                textPos.y += fontSz + pad;
            }
            const float step = fontSz + pad * 2;
            for (eecs::EntityId reply : dialogue_replies)
            {
                eecs::query_components(reg, reply, [&](const std::string& text, const std::vector<std::string>& reply_triggers)
                {
                    draw_button_9rect(nrect, Rectangle(textPos.x, textPos.y, sz.x * 0.3f, step), textFont, text.c_str(), 16.f, 0, scaleFactor, ColorFromHSV(0, 0, 0.7f),
                    [&]()
                    {
                        for (const std::string& trName : reply_triggers)
                            eecs::emit_event(reg, fnv1StrHash(trName.c_str()), eid, reply);
                    });
                    textPos.x += sz.x * 0.3f + pad;
                }, COMPID(const std::string, text), COMPID(const std::vector<std::string>, reply_triggers));
            }
        }, COMPID(const int, cam_resWidth), COMPID(const float, cam_resMult));
    }, COMPID(const std::string, dialogue_title), COMPID(const std::vector<std::string>, dialogue_text), COMPID(const std::vector<eecs::EntityId>, dialogue_replies), COMPID(Tag, dialogue_active));
}

void draw_screen_change(float width, float height, float scaleFactor, float ratio)
{
    int psz = 32 * scaleFactor;
    float incl = 0.1f;
    for (int yy = 0; yy < height / psz; ++yy)
    {
        float offs = yy % 2 ? 0.f : (incl * (-width/psz));
        float dir = yy % 2 ? -1.f : 1.f;
        for (int xx = 0; xx < width / psz; ++xx)
        {
            Color col = ColorFromNormalized({0.f, 0.f, 0.f, clamp(offs + dir * xx * incl + ratio * 5.f, 0.f, 1.f)});
            DrawRectangle(xx * psz, yy * psz, psz, psz, col);
        }
    }
}

void draw_ui(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    static Font headerFont = LoadFontEx("res/textures/ui/16px-IBM_VGA_8x16.ttf", 16, nullptr, 0);
    static Font logFont = LoadFontEx("res/textures/ui/8px-IBM_BIOS_8x8.ttf", 8, nullptr, 0);
    static NineRect nrect = create_9rect(LoadImage("res/textures/ui/button_rect.png"), 2);
    const float pad = 4.f * scaleFactor;
    if (gameState == E_MAIN_MENU)
    {
        vec2f bsz = {width * 0.5f, 40.f * scaleFactor};
        vec2f pos = {(width - bsz.x) * 0.5f, height * 0.5f};
        draw_button_9rect(nrect, Rectangle(pos.x, pos.y, bsz.x, bsz.y), headerFont, "Begin", 32.f, 0, scaleFactor, ColorFromHSV(0, 0, 0.7f),
        [&]()
        {
            eecs::create_or_find_entity_wrap(reg, "Switch_Level").set(COMPID(std::string, nextLevel), std::string("Meatspace_Main"));
            gameState = E_GAME;
        });
        pos.y += bsz.y + pad;
        draw_button_9rect(nrect, Rectangle(pos.x, pos.y, bsz.x, bsz.y), headerFont, "Logout", 32.f, 0, scaleFactor, ColorFromHSV(0, 0, 0.7f),
        [&]()
        {
            eecs::create_entity_wrap(reg).set(COMPID(Tag, quitGame), Tag{});
        });
    }
    else if (gameState == E_WIN)
    {
        DrawRectangle(0, 0, width, height, BLACK);
        draw_centered_font_with_shadow(headerFont, "YOU WON", torect(0, 0, width, height * 0.2f), 32.f * scaleFactor, 3, GetColor(0x3e8948ff));
        const char* wonText[] =
        {
            "You escaped the facility",
            "And maybe you're not in the dreamspace",
            "But you don't know for sure",
            "Actually..."
        };
        vec2f bsz = {width * 0.5f, 40.f * scaleFactor};
        vec2f pos = {(width - bsz.x) * 0.5f, height * 0.3f};
        for (int i = 0; i < sizeof(wonText) / sizeof(wonText[0]); ++i)
        {
            DrawTextEx(headerFont, wonText[i], toRLVec2(pos), 16.f * scaleFactor, 0, GetColor(0x3e8948ff));
            pos.y += 16.f * scaleFactor + pad;
        }
        draw_button_9rect(nrect, Rectangle(pos.x, pos.y, bsz.x, bsz.y), headerFont, "Logout", 32.f, 0, scaleFactor, ColorFromHSV(0, 0, 0.7f),
        [&]()
        {
            eecs::create_entity_wrap(reg).set(COMPID(Tag, quitGame), Tag{});
        });
    }
    else if (gameState == E_LOSE)
    {
        const float timeToChange = 1.f;
        timeFromChange += GetFrameTime();
        if (timeFromChange < timeToChange)
        {
            draw_screen_change(width, height, scaleFactor, timeFromChange / timeToChange);
            return;
        }
        DrawRectangle(0, 0, width, height, BLACK);
        draw_centered_font_with_shadow(headerFont, "YOU ARE FOREVER LOST", torect(0, 0, width, height * 0.2f), 32.f * scaleFactor, 3, GetColor(0x3e8948ff));
        const char* loseText[] =
        {
            "You were forever lost in the facility.",
            "Nobody knows if your body is being utilized by masters of the facility.",
            "Maybe you're feeding the resident programs in the cyberspace.",
            "Or you're part of the dreamspace now, feeding the warden.",
            "Maybe someone else will meet you there..."
        };
        vec2f bsz = {width * 0.5f, 40.f * scaleFactor};
        vec2f pos = {(width - bsz.x) * 0.5f, height * 0.3f};
        for (int i = 0; i < sizeof(loseText) / sizeof(loseText[0]); ++i)
        {
            DrawTextEx(headerFont, loseText[i], toRLVec2(pos), 16.f * scaleFactor, 0, GetColor(0x3e8948ff));
            pos.y += 16.f * scaleFactor + pad;
        }
        pos.y += 16.f * scaleFactor + pad;
        draw_button_9rect(nrect, Rectangle(pos.x, pos.y, bsz.x, bsz.y), headerFont, "Logout", 32.f, 0, scaleFactor, ColorFromHSV(0, 0, 0.7f),
        [&]()
        {
            eecs::create_entity_wrap(reg).set(COMPID(Tag, quitGame), Tag{});
        });
    }
    else if (gameState == E_GAME)
    {
        static Texture2D border = LoadTexture("res/textures/ui/border.png");
        static Texture2D horzborder = LoadTexture("res/textures/ui/horz_border2.png");
        const float headerSize = 16.f * scaleFactor;

        float charEnd = (40 + 14 * 5) * scaleFactor;

        const float uiPxScale = 4.f * scaleFactor;
        const float borderHt = uiPxScale * 8.f;
        const float actEnd = charEnd + borderHt + headerSize + pad + 20.f * scaleFactor * 8;
        const float itmEnd = height - 180 * scaleFactor;

        eecs::query_entities(reg, [&](eecs::EntityId, int cam_resWidth, float cam_resMult)
        {
            const float leftSide = cam_resWidth * cam_resMult * scaleFactor - cam_resMult * scaleFactor;
            const float leftSideTrue = leftSide + cam_resMult * scaleFactor;
            draw_tiled_tex(horzborder, torect(0, 0, width, 12), torect(leftSideTrue, charEnd, width, 12 * scaleFactor), uiPxScale, WHITE);
            draw_tiled_tex(horzborder, torect(0, 0, width, 12), torect(leftSideTrue, actEnd, width, 12 * scaleFactor), uiPxScale, WHITE);
            draw_tiled_tex(horzborder, torect(0, 0, width, 12), torect(leftSideTrue, itmEnd, width, 12 * scaleFactor), uiPxScale, WHITE);

            draw_centered_font_with_shadow(headerFont, "CHARACTER", torect(leftSide + 12.f * uiPxScale, 16.f * scaleFactor, width - (leftSide + 12.f * uiPxScale), headerSize), headerSize, 3, GetColor(0x3e8948ff));
            draw_centered_font_with_shadow(headerFont, "ACTIONS", torect(leftSide + 12.f * uiPxScale, charEnd + borderHt, width - (leftSide + 12.f * uiPxScale), headerSize), headerSize, 3, GetColor(0x3e8948ff));
            draw_centered_font_with_shadow(headerFont, "ITEMS", torect(leftSide + 12.f * uiPxScale, actEnd + borderHt, width - (leftSide + 12.f * uiPxScale), headerSize), headerSize, 3, GetColor(0x3e8948ff));
            draw_centered_font_with_shadow(headerFont, "SYSLOG", torect(leftSide + 12.f * uiPxScale, itmEnd + borderHt, width - (leftSide + 12.f * uiPxScale), headerSize), headerSize, 3, GetColor(0x3e8948ff));

            draw_tiled_tex(border, torect(0, 0, 12, height), torect(leftSide, 0.f, 12 * scaleFactor, height), uiPxScale, WHITE);
        }, COMPID(const int, cam_resWidth), COMPID(const float, cam_resMult));

        draw_character(reg, 40.f * scaleFactor, width, height, scaleFactor);
        //DrawFPS(20, height - 40);
        draw_interactables(reg, charEnd + borderHt + headerSize + pad, width, height, scaleFactor);

        draw_items(reg, actEnd + borderHt + headerSize + pad, width, height, scaleFactor);

        const vec2i cam_wh = get_cam_wh(reg);
        vec2f pos = {cam_wh.x + 50.f * scaleFactor, itmEnd + borderHt + headerSize + pad};
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

        draw_dialogue(reg, width, height, scaleFactor);
    }
}

void push_rolling_text(eecs::Registry& reg, const char* text, Color col)
{
    eecs::query_component(reg, eecs::find_entity(reg, "rolling_text"), [&](std::vector<ColoredText>& rollingText)
    {
        rollingText.push_back(std::make_pair(std::string(text), col));
    }, COMPID(std::vector<ColoredText>, rollingText));
}

