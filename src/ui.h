#pragma once
#include "podval.h"
#include <raylib.h>
#include <array>

constexpr Color selectedColor = Color{240, 230, 0, 220};
constexpr Color notSelectedColor = Color{120, 120, 120, 220};

enum FontCentering
{
  EFC_HCENTER = 1,
  EFC_VCENTER = 2
};

void draw_font_with_shadow(Font font, const char* text, float x, float y, float size, Color col);
float draw_bounded_font_with_shadow(Font font, const char* text, float x, float y, float width, float size, float pad, Color col);
void draw_centered_font_with_shadow(Font font, const char* text, Rectangle rect, float size, Color col, FontCentering fc = FontCentering(EFC_HCENTER | EFC_VCENTER));
void draw_centered_block_with_shadow(Font font, int num, const char** text, Rectangle rect, float size, Color col);
void draw_centered_texture(Texture2D tex, Rectangle rect, float scale);

void draw_button(Rectangle rect, const char* text, float scaleFactor, Color col);

void draw_ui(eecs::Registry& reg, flecs::world& ecs, float width, float height, float scaleFactor);

void draw_game_ui(flecs::world& ecs);

void register_ui(flecs::world& ecs);
void create_ui_helper(eecs::Registry& reg, flecs::world& ecs, float width, float height, float scaleFactor);

struct WindowWidth : public FVal {};
struct WindowHeight : public FVal {};
struct WindowScaleFactor : public FVal {};

template<typename Vec>
inline bool is_vec_in_rect(const Vec& v, Rectangle r) { return v.x >= r.x && v.y >= r.y && v.x <= r.x + r.width && v.y <= r.y + r.height; }

bool is_cursor_over_ui(flecs::world& ecs);
bool is_ui_blocks_input();

struct NineRect
{
  std::array<Texture2D, 9> rects;
  int sz;
};
NineRect create_9rect(Image img, int sz);
void draw_9rect(const NineRect& nr, Rectangle rect, float scale, Color col);

void draw_button_9rect(const NineRect& nrect, Rectangle rect, Font font, const char* text, float fontSize, float scaleFactor, Color col);

