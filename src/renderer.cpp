#include <raylib.h>
#include <flecs.h>
#include <eecs.h>
#include <algorithm>
#include <random>

#include "renderer.h"
#include "movement.h"
#include "raymath.h"
#include "level.h"
#include "game.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif
static Shader lightingShader;
static Light lights[MAX_LIGHTS];

struct renderer
{
  Shader instancingShader;
  Material instancingMaterial;
  Mesh cube;
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_real_distribution<> dis;

  renderer(flecs::world& ecs)
    : gen(rd()),
      dis(0.0, 1.0)
  {
    ecs.module<renderer>();

    cube = GenMeshCube(1.f, 1.f, 1.f);
    ecs.component<LightStrength>()
      .member<float>("val");

    //ecs.component<Texture2D>();
    /*
    ecs.component<RenderTexture2D>()
      .opaque(ecs.component()
            .member<int>("width")
            .member<int>("height"))
        .serialize([](const flecs::serializer *s, const RenderTexture2D *data) {
            s->member("width");
            s->value(data->texture.width);
            s->member("height");
            s->value(data->texture.height);
            return 0;
        })
        .ensure_member([](RenderTexture2D *dst, const char *member) -> void* {
            if (!strcmp(member, "width")) {
                return &dst->texture.width;
            } else if (!strcmp(member, "height")) {
                return &dst->texture.height;
            } else {
                return nullptr; // We can't serialize into fake result member
            }
        });
        */

    lightingShader = LoadShader(TextFormat("res/shaders/glsl%i/lighting.vs", GLSL_VERSION),
                               TextFormat("res/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    instancingShader = LoadShader(TextFormat("res/shaders/glsl%i/lighting_instancing.vs", GLSL_VERSION),
                               TextFormat("res/shaders/glsl%i/base.fs", GLSL_VERSION));
    lightingShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(lightingShader, "viewPos");
    lightingShader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(lightingShader, "emissiveMap");
    instancingShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(instancingShader, "viewPos");

    instancingMaterial = LoadMaterialDefault();
    instancingMaterial.shader = instancingShader;
    int ambientLoc = GetShaderLocation(lightingShader, "ambient");
    float ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    SetShaderValue(lightingShader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);

    memset(lights, 0, sizeof(Light) * MAX_LIGHTS);
  }
};

void register_renderer(eecs::Registry& reg, flecs::world& ecs)
{
    ecs.import<renderer>();
    // Move to render actually!
    eecs::reg_system(reg, [&](eecs::EntityId, const Camera& camera)
    {
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(lightingShader, lightingShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        lightsCount = 0;
        int nl = 0;
        eecs::query_entities(reg, [&](eecs::EntityId, float light_strength, const vec3f& position)
        {
            if (nl >= MAX_LIGHTS)
                return;
            lights[nl] = CreateLight(LIGHT_POINT, toRLVec3(position), Vector3Zero(), Vector4{1.f, 1.f, 1.f, light_strength}, lightingShader);
            nl++;
        }, COMPID(const float, light_strength), COMPID(const vec3f, position));
        eecs::query_entities(reg, [&](eecs::EntityId eid, Model& model, const vec3f& position)
        {
            model.materials[0].shader = lightingShader;
            DrawModelEx(model, toRLVec3(position), Vector3{0.f, 1.f, 0.f}, eecs::get_comp_or(reg, eid, COMPID(float, rotation), 0.f), Vector3{1.f, 1.f, 1.f}, WHITE);
        }, COMPID(Model, model), COMPID(const vec3f, position));
    }, COMPID(const Camera, camera));
}

void pre_draw_call(eecs::Registry& reg, flecs::world& ecs)
{
}

