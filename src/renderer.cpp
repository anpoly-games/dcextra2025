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

void register_renderer(flecs::world& ecs)
{
    ecs.import<renderer>();
    // Move to render actually!
    ecs.system<const Camera>()
    .each([&](const Camera& cam)
    {
        float cameraPos[3] = { cam.position.x, cam.position.y, cam.position.z };
        SetShaderValue(lightingShader, lightingShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        lightsCount = 0;
        auto lightQ = ecs.query<const LightStrength, const Position>();
        int nl = 0;
        lightQ.each([&](const LightStrength& light, const Position& lpos)
        {
            if (nl >= MAX_LIGHTS)
                return;
            lights[nl] = CreateLight(LIGHT_POINT, toRLVec3(lpos), Vector3Zero(), Vector4{1.f, 1.f, 1.f, light.val}, lightingShader);
            nl++;
        });
        ecs.query<const Model, const Position, const Rotation*>()
        .each([&](const Model& model, const Position& position, const Rotation* rot)
        {
            model.materials[0].shader = lightingShader;
            DrawModelEx(model, toRLVec3(position), Vector3{0.f, 1.f, 0.f}, rot ? rot->val : 0.f, Vector3{1.f, 1.f, 1.f}, WHITE);
        });
    });
}

void pre_draw_call(eecs::Registry& reg, flecs::world& ecs)
{
}

