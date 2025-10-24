#include <eecs.h>
#include <raylib.h>
#include <filesystem>

#include "textures.h"

namespace fs = std::filesystem;

/*
flecs::opaque<std::string> std_string_support(flecs::world&)
{
  flecs::opaque<std::string> ts;

  // Let reflection framework know what kind of type this is
  ts.as_type(flecs::String);

  // Forward std::string value to (JSON/...) serializer
  ts.serialize([](const flecs::serializer *s, const std::string *data)
  {
    const char *value = data->c_str();
    return s->value(flecs::String, &value);
  });

  // Serialize string into std::string
  ts.assign_string([](std::string *data, const char *value)
  {
    *data = value;
  });

  return ts;
}
*/

/*
struct textures
{
  textures(flecs::world& ecs)
  {
    ecs.module<textures>();

    ecs.component<std::string>("String")
      .opaque(std_string_support);

    ecs.component<TextureFile>()
      .on_set([&](flecs::entity ent, const TextureFile& tf)
      {
        fs::path path{tf.filename};
        Texture2D tex = LoadTexture(tf.filename.c_str());
        ent.set(tex);

        fs::path emissivePath = path.parent_path() / (path.stem().string() + "_emissive" + path.extension().string());
        Texture2D emissive = LoadTexture(emissivePath.string().c_str());
        ent.set(EmissiveTex{emissive});
        ent.get([&](const Model& model)
        {
          if (model.materialCount > 0)
          {
            printf("load textures for model %d materials\n", model.materialCount);
            SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, tex);
            SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, emissive);
          }
        });
        ent.get([&](const Mesh& mesh)
        {
          Model model = LoadModelFromMesh(mesh);
          printf("load textures for mesh %d materials\n", model.materialCount);
          SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, tex);
          SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, emissive);
          ent.set(model);
        });
      })
      .member<std::string>("filename")
      .add(flecs::OnInstantiate, flecs::Inherit);
  }
};
*/

void register_textures(eecs::Registry& reg)
{
  //ecs.import<textures>();
}

