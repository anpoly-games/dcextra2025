#include <raylib.h>
#include <eecs.h>
#include <cstring>

#include "primitives.h"
#include "textures.h"
#include "math.h"

static void fix_cube_coords(Mesh& mesh, const vec3f& tcMult)
{
  if (mesh.texcoords != nullptr)
  {
    float texcoords[] = {
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

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

        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f
    };
    for (int i = 0; i < 8 * 2 * 1; ++i)
        texcoords[i] *= tcMult.x * tcMult.y;
    for (int i = 8 * 2 * 1; i < 8 * 2 * 2; ++i)
        texcoords[i] *= tcMult.x * tcMult.z;
    for (int i = 8 * 2 * 2; i < 8 * 2 * 3; ++i)
        texcoords[i] *= tcMult.y * tcMult.z;
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
        const vec3f tcMult = eecs::get_comp_or(reg, eid, COMPID(vec3f, primitiveBlock_tcMult), primitiveBlock_size);
        fix_cube_coords(mesh, tcMult);
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

