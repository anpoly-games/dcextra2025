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

struct renderer
{
  Shader lightingShader;
  Shader instancingShader;
  Material instancingMaterial;
  Mesh cube;
  Light lights[MAX_LIGHTS];
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
    ecs.system<const NumParticles>()
      .without<Particles>()
      .each([&](flecs::entity e, const NumParticles& np)
      {
        e.set(Particles());
      });
    ecs.system<Particles, const NumParticles, const Position>()
      .each([&](Particles& particles, const NumParticles& num, const Position& pos)
      {
        // spawn random particles in the zone
        while (particles.particles.size() < num.val)
        {
          const float px = dis(gen) - 0.5f;
          const float py = dis(gen);
          const float pz = dis(gen) - 0.5f;
          particles.particles.push_back(vec3f{pos.x + px, pos.y + py, pos.z + pz});
        }
        const float spd = 5.f;
        for (vec3f& part : particles.particles)
        {
          part.y += get_game_dt(ecs) * spd;
          while (part.y > 1.f)
          {
            part.y -= 1.f;
            part.x = pos.x + dis(gen) - 0.5f;
            part.z = pos.z + dis(gen) - 0.5f;
          }
        }
      });
    ecs.system()
      .each([&]()
      {
        size_t numParticles = 0;
        ecs.query<const Particles>().each([&](const Particles& particles) { numParticles += particles.particles.size(); } );
        if (numParticles == 0)
          return;
        std::vector<Matrix> matrices;
        matrices.resize(numParticles);
        Matrix* mats = matrices.data();
        auto partQ = ecs.query<const Particles, const ParticleColor, const ParticleSize>();
        size_t i = 0;
        partQ.each([&](const Particles& particles, const ParticleColor& col, const ParticleSize& sz)
        {
          for (const vec3f& part : particles.particles)
          {
            Matrix translation = MatrixTranslate(part.x, part.y, part.z);
            Matrix scale = MatrixScale(sz.val, sz.val * 3.f, sz.val);
            mats[i] = MatrixMultiply(scale, translation);
            mats[i].m3 = col.x;
            mats[i].m7 = col.y;
            mats[i].m11 = col.z;
            mats[i].m15 = col.w;
            i++;
          }
        });
        DrawMeshInstanced(cube, instancingMaterial, mats, numParticles);
      });
  }
};

void register_renderer(flecs::world& ecs)
{
  ecs.import<renderer>();
}

void pre_draw_call(eecs::Registry& reg, flecs::world& ecs)
{
  const renderer& rend = ecs.import<renderer>().get<renderer>();
  flecs::entity player = ecs.lookup("player");
  if (!player)
    return;
  flecs::entity inv = player.lookup("inventory");
  if (!inv)
    return;
  ecs.defer_begin();
  inv.children([&](flecs::entity e)
  {
    lightsCount = 0;
    Light light;
    memset(&light, 0, sizeof(Light));
    light = CreateLight(LIGHT_POINT, Vector3{0.f, 0.f, -1.f}, Vector3Zero(), Vector4{1.f, 1.f, 1.f, 1.f}, rend.lightingShader);
    UpdateLightValues(rend.lightingShader, light);
    e.insert([&](RenderTexture2D& target, const Model& model)
    {
      const float fovy = 90.f;
      BoundingBox bb = GetModelBoundingBox(model);
      const float bbhh = std::max(bb.max.y, -bb.min.y);
      const float bbhw = std::max(std::max(bb.max.z, -bb.min.z), std::max(bb.max.x, -bb.min.x));
      const float dist = std::max(bbhh * 1.2f / tanf(fovy * 0.5f * DEG2RAD), bbhw * 1.2f) * 2.f;
      BeginTextureMode(target);
        ClearBackground(Color{0, 0, 0, 0});
        Camera3D modelCam = { {cosf(GetTime()) * dist, 0.f, sinf(GetTime()) * dist}, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, fovy, 0 };
        BeginMode3D(modelCam);
          DrawModel(model, Vector3Zero(), 1.f, WHITE);
        EndMode3D();
      EndTextureMode();
    });
  });
  ecs.defer_end();
}

