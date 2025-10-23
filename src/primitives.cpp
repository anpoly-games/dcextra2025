#include <raylib.h>
#include <flecs.h>

#include "primitives.h"
#include "textures.h"

static void fix_cube_coords(Mesh& mesh)
{
  if (mesh.texcoords != nullptr)
  {
    float texcoords[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };
    memcpy(mesh.texcoords, texcoords, 24*2*sizeof(float));
    UpdateMeshBuffer(mesh, SHADER_LOC_VERTEX_TEXCOORD01, mesh.texcoords, sizeof(float) * 2 * mesh.vertexCount, 0);
  }
}

struct primitives
{
  primitives(flecs::world& ecs)
  {
    ecs.module<primitives>();

    ecs.component<RelativePos>()
      .member<vec3f>("val");

    ecs.component<Cube>()
      .on_set([&](flecs::entity ent, const Cube& cube)
      {
        Mesh mesh = GenMeshCube(cube.sz.x, cube.sz.y, cube.sz.z);
        fix_cube_coords(mesh);
        ent.set(mesh);
        ent.get([&](const Texture2D& tex, const EmissiveTex& emissive)
        {
          Model model = LoadModelFromMesh(mesh);
          SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, tex);
          SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, emissive.tex);
          ent.set(model);
        });
      })
      .member<vec3f>("sz");

    ecs.component<Billboard>()
      .member<vec2f>("sz");

    /*
    ecs.component<Model>()
      .add(flecs::OnInstantiate, flecs::Inherit);
    ecs.component<Mesh>()
      .add(flecs::OnInstantiate, flecs::Inherit);
      */
    ecs.component<ModelFile>()
      .on_set([&](flecs::entity ent, const ModelFile& mf)
      {
        Model model = LoadModel(mf.filename.c_str());
        ent.get([&](const Texture2D& tex, const EmissiveTex& emissive)
        {
          printf("load model %d materials\n", model.materialCount);
          SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, tex);
          SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, emissive.tex);
        });
        ent.set(model);
      })
      .member<std::string>("filename")
      .add(flecs::OnInstantiate, flecs::Inherit);
  }
};

void register_primitives(flecs::world& ecs)
{
  ecs.import<primitives>();

  ecs_script_run_file(ecs, "res/prefabs/primitives.flecs");
}

