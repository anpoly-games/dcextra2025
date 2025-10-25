#include <raylib.h>
#include <eecs.h>
#include <cstring>

#include "primitives.h"
#include "textures.h"
#include "math.h"

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

void register_primitives(eecs::Registry& reg)
{
    // TODO: move somewhere
    static Image defEmissiveImage = GenImageColor(1, 1, BLACK);
    static Texture2D defEmissiveTex = LoadTextureFromImage(defEmissiveImage);
    eecs::reg_enter(reg, [&](eecs::EntityId eid, const vec3f& primitiveBlock_size)
    {
        Mesh mesh = GenMeshCube(primitiveBlock_size.x, primitiveBlock_size.y, primitiveBlock_size.z);
        fix_cube_coords(mesh);
        eecs::EntityWrap ent = eecs::wrap_entity(reg, eid);
        Model model = LoadModelFromMesh(mesh);
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, defEmissiveTex);
        ent.set(COMPID(Model, model), model);
    }, COMPID(const vec3f, primitiveBlock_size));

    eecs::reg_enter(reg, [&](eecs::EntityId eid, const std::string& model_filename)
    {
        Model model = LoadModel(model_filename.c_str());
        eecs::EntityWrap ent = eecs::wrap_entity(reg, eid);
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, defEmissiveTex);
        ent.set(COMPID(Model, model), model);
    }, COMPID(const std::string, model_filename));

    eecs::reg_enter(reg, [&](eecs::EntityId eid, Model& model, const Texture2D& texture_diff)
    {
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, texture_diff);
    }, COMPID(Model, model), COMPID(const Texture2D, texture_diff));
    eecs::reg_enter(reg, [&](eecs::EntityId eid, Model& model, const Texture2D& texture_emissive)
    {
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_EMISSION, texture_emissive);
    }, COMPID(Model, model), COMPID(const Texture2D, texture_emissive));
}

