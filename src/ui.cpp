#include <raylib.h>
#include <flecs.h>
#include <eecs.h>
#include <algorithm>
#include <string>

#include "ui.h"
#include "math.h"

void draw_centered_texture(Texture2D tex, Rectangle rect, float scale)
{
  Vector2 pos = Vector2{rect.x + (rect.width - tex.width * scale) * 0.5f, rect.y + (rect.height - tex.height * scale) * 0.5f};
  DrawTextureEx(tex, pos, 0.f, scale, WHITE);
}

void draw_font_with_shadow(Font font, const char* text, float x, float y, float size, Color col)
{
  Color shadowC = BLACK;
  shadowC.a = col.a;
  DrawTextEx(font, text, tovec(x - 1, y - 1), size, 3, shadowC);
  DrawTextEx(font, text, tovec(x + 1, y + 1), size, 3, shadowC);
  DrawTextEx(font, text, tovec(x - 1, y + 1), size, 3, shadowC);
  DrawTextEx(font, text, tovec(x + 1, y - 1), size, 3, shadowC);

  DrawTextEx(font, text, tovec(x - 1, y), size, 3, shadowC);
  DrawTextEx(font, text, tovec(x + 1, y), size, 3, shadowC);
  DrawTextEx(font, text, tovec(x, y + 1), size, 3, shadowC);
  DrawTextEx(font, text, tovec(x, y - 1), size, 3, shadowC);

  DrawTextEx(font, text, tovec(x, y), size, 3, col);
}

float draw_bounded_font_with_shadow(Font font, const char* text, float x, float y, float width, float size, float pad, Color col)
{
  std::string str = text;
  size_t start = 0;
  size_t next = str.find_first_of(" ", start);
  float xpos = x;
  float ypos = y;
  do
  {
    if (next == std::string::npos)
    {
      std::string sub = str.substr(start) + " ";
      Vector2 sz = MeasureTextEx(font, sub.c_str(), size, 3);
      if (xpos + sz.x > width)
      {
        xpos = x;
        ypos += sz.y + pad;
      }
      draw_font_with_shadow(font, sub.c_str(), xpos, ypos, size, col);
      xpos += sz.x;
      break;
    }
    std::string sub = str.substr(start, next - start) + " ";
    Vector2 sz = MeasureTextEx(font, sub.c_str(), size, 3);
    if (xpos + sz.x > width)
    {
      xpos = x;
      ypos += sz.y + pad;
    }
    draw_font_with_shadow(font, sub.c_str(), xpos, ypos, size, col);
    xpos += sz.x;
    start = next + 1;
    next = str.find_first_of(" ", start);
  } while (true);
  return ypos + size + pad;
}

void draw_centered_font_with_shadow(Font font, const char* text, Rectangle rect, float size, Color col, FontCentering fc)
{
  const bool vcenter = fc & EFC_VCENTER;
  const bool hcenter = fc & EFC_HCENTER;
  Vector2 sz = MeasureTextEx(font, text, size, 3);
  draw_font_with_shadow(font, text, rect.x + (hcenter ? (rect.width - sz.x) * 0.5f : 0), rect.y + (vcenter ? (rect.height - sz.y) * 0.5f : 0), size, col);
}

void draw_centered_block_with_shadow(Font font, int num, const char** text, Rectangle rect, float size, Color col)
{
  const Vector2 sz = MeasureTextEx(font, text[0], size, 3);
  const float lineHt = sz.y;
  float vstart = rect.y + (rect.height - lineHt * num) * 0.5f;
  for (int i = 0; i < num; ++i)
    draw_centered_font_with_shadow(font, text[i], torect(rect.x, vstart + lineHt * i, rect.width, lineHt), size, col);
}

void draw_button(Rectangle rect, const char* text, float scaleFactor, Color col)
{
  Vector2 mp = GetMousePosition();
  if (is_vec_in_rect(mp, rect))
  {
    Vector3 hsv = ColorToHSV(col);
    col = ColorFromHSV(hsv.x, hsv.y, std::min(1.f, hsv.z + 0.3f));
  }
  DrawRectangleRec(rect, col);
  draw_centered_font_with_shadow(GetFontDefault(), text, rect, 12.f * scaleFactor, WHITE);
}

void draw_button_9rect(const NineRect& nr, Rectangle rect, Font font, const char* text, float fontSize, float scaleFactor, Color col)
{
  Vector2 mp = GetMousePosition();
  if (is_vec_in_rect(mp, rect))
  {
    Vector3 hsv = ColorToHSV(col);
    const char alpha = col.a;
    col = ColorFromHSV(hsv.x, hsv.y, std::min(1.f, hsv.z + 0.3f));
    col.a = alpha;
  }
  draw_9rect(nr, rect, scaleFactor, col);
  draw_centered_font_with_shadow(font, text, rect, fontSize, Color{255, 255, 255, col.a});
}

void register_ui(flecs::world& ecs)
{
  // do nothing yet
}

void create_ui_helper(eecs::Registry& reg, float width, float height, float scaleFactor)
{
    eecs::create_entity_wrap(reg)
        .set(COMPID(float, window_width), width)
        .set(COMPID(float, window_height), height)
        .set(COMPID(float, window_scaleFactor), scaleFactor);
}

NineRect create_9rect(Image img, int sz)
{
  NineRect res;
  res.sz = sz;
  res.rects[0] = LoadTextureFromImage(ImageFromImage(img, torect(0, 0, sz, sz)));
  res.rects[1] = LoadTextureFromImage(ImageFromImage(img, torect(sz, 0, img.width - 2 * sz, sz)));
  res.rects[2] = LoadTextureFromImage(ImageFromImage(img, torect(img.width - sz, 0, sz, sz)));

  res.rects[3] = LoadTextureFromImage(ImageFromImage(img, torect(0, sz, sz, img.height - 2 * sz)));
  res.rects[4] = LoadTextureFromImage(ImageFromImage(img, torect(sz, sz, img.width - 2 * sz, img.height - 2 * sz)));
  res.rects[5] = LoadTextureFromImage(ImageFromImage(img, torect(img.width - sz, sz, sz, img.height - 2 * sz)));

  res.rects[6] = LoadTextureFromImage(ImageFromImage(img, torect(0, img.width - sz, sz, sz)));
  res.rects[7] = LoadTextureFromImage(ImageFromImage(img, torect(sz, img.width - sz, img.width - 2 * sz, sz)));
  res.rects[8] = LoadTextureFromImage(ImageFromImage(img, torect(img.width - sz, img.width - sz, sz, sz)));

  for (Texture2D& tex : res.rects)
  {
    //SetTextureWrap(tex, TEXTURE_WRAP_REPEAT);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);
  }
  return res;
}

void draw_tiled_tex(Texture2D tex, Rectangle source, Rectangle dest, float scale, Color col)
{
  float x = 0;
  while (x < source.width)
  {
    float ww = std::min((float)tex.width, (float)source.width - x);
    float y = 0;
    while (y < source.height)
    {
      float hh = std::min((float)tex.height, (float)source.height - y);
      DrawTexturePro(tex, torect(0, 0, ww, hh),
          torect(dest.x + x * scale, dest.y + y * scale, ww * scale, hh * scale), Vector2{0, 0}, 0, col);
      y += hh;
    }
    x += ww;
  }
}

void draw_9rect(const NineRect& nr, Rectangle rect, float scale, Color col)
{
  Vector2 csz = Vector2{rect.width / scale - nr.sz * 2, rect.height / scale - nr.sz * 2};
  std::array<float, 3> hzSz =   {(float)nr.sz, (float)csz.x, (float)nr.sz};
  std::array<float, 3> vertSz = {(float)nr.sz, (float)csz.y, (float)nr.sz};
  std::array<float, 3> hzGrid =
  {
    rect.x,
    rect.x + nr.sz * scale,
    rect.x + nr.sz * scale + csz.x * scale
  };
  std::array<float, 3> vertGrid =
  {
    rect.y,
    rect.y + nr.sz * scale,
    rect.y + nr.sz * scale + csz.y * scale
  };

  for (size_t i = 0; i < 9; ++i)
  {
    size_t x = i % 3;
    size_t y = i / 3;
    draw_tiled_tex(nr.rects[i], torect(0, 0, hzSz[x], vertSz[y]), torect(hzGrid[x], vertGrid[y], hzSz[x] * scale, vertSz[y] * scale), scale, col);
  }
}

