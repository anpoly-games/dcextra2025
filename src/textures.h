#pragma once

#include <string>

struct TextureFile
{
  std::string filename;
};

struct EmissiveTex
{
  Texture2D tex;
};

void register_textures(eecs::Registry& reg);

