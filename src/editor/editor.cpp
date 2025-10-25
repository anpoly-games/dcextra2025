#include <raylib.h>
#include <eecs.h>
#include <edat.h>
#include <reflection.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <raymath.h>
#include <rlgl.h>

#include "editor.h"
#include "editor_ui.h"
#include "../game.h"
#include "../math.h"
#include "../renderer.h"
#include "../primitives.h"
#include "../player.h"
#include "../level.h"
#include "../ui.h"
#include "../cam.h"
#include "../textures.h"
#include "../tags.h"
#include "../audio.h"

namespace fs = std::filesystem;

void register_systems(eecs::Registry& reg)
{
  //register_math(ecs);
  register_textures(reg);
  register_primitives(reg);
  register_player(reg);
  register_audio(reg);
  register_renderer(reg);
  register_cam(reg);
  register_level(reg);
  register_editor(reg);
  register_ui(reg);
}

eecs::EntityId init_new_world(eecs::Registry& reg)
{
  create_cam(reg);
  return create_player(reg);
}

void restart_world(eecs::Registry& reg)
{
    float width, height, scaleFactor;
    eecs::query_entities(reg, [&](eecs::EntityId, float window_width, float window_height, float window_scaleFactor)
    {
        width = window_width;
        height = window_height;
        scaleFactor = window_scaleFactor;
    }, COMPID(const float, window_width), COMPID(const float, window_height), COMPID(const float, window_scaleFactor));

    eecs::del_all_entities(reg);
    eecs::del_all_systems(reg);
    register_systems(reg);
    init_new_world(reg);
    create_ui_helper(reg, width, height, scaleFactor);
}


void create_floor(eecs::Registry& reg, vec3f tilePos, float rot, const char* name);
void create_ceiling(eecs::Registry& reg, int x, int y, float rot, const char* name);
void create_wall(eecs::Registry& reg, vec3f tilePos, int dir, bool flip, const char* name);
void create_door(eecs::Registry& reg, int x, int y, int dir, const char* name);
void create_column(eecs::Registry& reg, int x, int y, const char* name);
void create_entity(eecs::Registry& reg, int x, int y, float rot, const char* name);
void create_logic(eecs::Registry& reg, int x, int y, float rot, const char* name);

void delete_floor(eecs::Registry& reg, vec3f tilePos);
void delete_ceiling(eecs::Registry& reg, int x, int y);
void delete_wall(eecs::Registry& reg, vec3f tilePos, int dir);
void delete_door(eecs::Registry& reg, int x, int y, int dir);
void delete_column(eecs::Registry& reg, int x, int y);

static void draw_cube_matrix(const BoundingBox& bbox, const vec3f& pos, float rotation, Color col)
{
  Matrix matRotation = MatrixRotate(Vector3{0.f, 1.f, 0.f}, rotation*DEG2RAD);
  Matrix matTranslation = MatrixTranslate(pos.x, pos.y, pos.z);

  Matrix matTransform = MatrixMultiply(matRotation, matTranslation);
  Vector3 vertices[8];
  int idx = 0;
  for (float z : {bbox.min.z, bbox.max.z})
    for (float y : {bbox.min.y, bbox.max.y})
      for (float x : {bbox.min.x, bbox.max.x})
        vertices[idx++] = Vector3{x, y, z} * matTransform;
  /*
       3+----+7
       /|   /|
      / |  / |
    2+----+6 |
     | 1+-|--+5
     | /  | /
     |/   |/
    0+----+4

     y
     ^ /z
     |/
     o-->x
  */
  const int indices[] =
  {
    0, 4, 6,
    0, 6, 2,
    1, 0, 2,
    1, 2, 3,
    4, 5, 7,
    4, 7, 6,
    0, 1, 5,
    0, 5, 4,
    1, 3, 7,
    1, 7, 5,
    2, 6, 7,
    2, 7, 3
  };
  for (size_t i = 0; i < sizeof(indices) / sizeof(indices[0]); i += 3)
    DrawTriangle3D(vertices[indices[i]], vertices[indices[i+1]], vertices[indices[i+2]], col);

  const int lineIndices[] =
  {
    0, 4,
    4, 6,
    6, 2,
    2, 0,
    4, 5,
    5, 7,
    7, 6,
    7, 3,
    3, 1,
    1, 5,
    3, 2,
    1, 0
  };
  for (size_t i = 0; i < sizeof(lineIndices) / sizeof(lineIndices[0]); i += 2)
    DrawLine3D(vertices[lineIndices[i]], vertices[lineIndices[i+1]], Color{col.r, col.g, col.b, 255});
}

Vector3 Vector3Rotate(Vector3 v, Matrix mat)
{
    Vector3 result = { 0 };

    float x = v.x;
    float y = v.y;
    float z = v.z;

    result.x = mat.m0*x + mat.m4*y + mat.m8*z;
    result.y = mat.m1*x + mat.m5*y + mat.m9*z;
    result.z = mat.m2*x + mat.m6*y + mat.m10*z;

    return result;
}

void expand_box(BoundingBox& bbox, float amt)
{
  bbox.min.x -= amt;
  bbox.min.y -= amt;
  bbox.min.z -= amt;
  bbox.max.x += amt;
  bbox.max.y += amt;
  bbox.max.z += amt;
}

void register_editor(eecs::Registry& reg)
{
    eecs::reg_system(reg, [&](eecs::EntityId eid, const Camera& camera)
    {
        const vec3f cpos = tov3(camera.position);
        const vec3i pivot = vec3i(cpos.x, 0, cpos.z);
        const int numTiles = 10;
        const vec3f minP(pivot.x - numTiles - 0.5f, 0.f, pivot.z - numTiles - 0.5f);
        const vec3f maxP(pivot.x + numTiles - 0.5f, 0.f, pivot.z + numTiles - 0.5f);
        const float plFloor = float(floorf(cpos.y));
        for (int y = pivot.z - numTiles; y <= pivot.z + numTiles; ++y)
        {
            DrawLine3D(Vector3{minP.x, -0.01f, y - 0.5f}, Vector3{maxP.x, -0.01f, y - 0.5f}, WHITE);
            DrawLine3D(Vector3{minP.x, 1.01f, y - 0.5f}, Vector3{maxP.x, 1.01f, y - 0.5f}, WHITE);
        }
        for (int x = pivot.x - numTiles; x <= pivot.x + numTiles; ++x)
        {
            DrawLine3D(Vector3{x - 0.5f, -0.01f, minP.z}, Vector3{x - 0.5f, -0.01f, maxP.z}, WHITE);
            DrawLine3D(Vector3{x - 0.5f, 1.01f, minP.z}, Vector3{x - 0.5f, 1.01f, maxP.z}, WHITE);
        }

        if (is_cursor_over_ui(reg))
            return;
        Ray r = GetScreenToWorldRay(GetMousePosition(), camera);
        const vec3f rp = tov3(r.position);
        const vec3f rd = tov3(r.direction);

        const vec3f camDir = tov3(camera.target) - tov3(camera.position);
        const float camRot = 270 - floorf((atan2f(camDir.z, camDir.x) * 180.f / PI) / 90.f + 0.5f) * 90.f;

        // TODO: Go through all level geometry first
        // find floor intersection
        eecs::query_entities(reg, [&](eecs::EntityId eid, eecs::EntityId& selectedEntity)
        {
            if (selectedEntity == eecs::invalid_eid)
                return;
            if (IsMouseButtonReleased(1))
                selectedEntity = eecs::invalid_eid;
            eecs::EntityWrap sel = eecs::wrap_entity(reg, selectedEntity);
            if (sel.has(COMPID(Tag, Door)) || sel.has(COMPID(Tag, wall)))
                return;
            // Draw gizmos
            const float rot = sel.get_or(COMPID(float, rotation), 0.f);
            sel.query_comp([&](vec3f& position)
            {
                Matrix matRotation = MatrixRotate(Vector3{0.f, 1.f, 0.f}, rot * DEG2RAD);
                Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

                Matrix matTransform = MatrixMultiply(matRotation, matTranslation);
                Vector3 pivot = Vector3{0.f, 0.f, 0.f} * matTransform;
                EndMode3D();
                Vector3 axes[3] = { Vector3{0.5f, 0.f, 0.f} * matTransform, Vector3{0.f, 0.5f, 0.f} * matTransform, Vector3{0.f, 0.f, 0.5f} * matTransform };
                vec2f p2d = tov2(GetWorldToScreen(pivot, camera));
                vec2f x2d = tov2(GetWorldToScreen(axes[0], camera));
                vec2f y2d = tov2(GetWorldToScreen(axes[1], camera));
                vec2f z2d = tov2(GetWorldToScreen(axes[2], camera));
                vec2f mp = tov2(GetMousePosition());
                float d2x = point_to_segm_dist(mp, p2d, x2d);
                float d2y = point_to_segm_dist(mp, p2d, y2d);
                float d2z = point_to_segm_dist(mp, p2d, z2d);
                float minDist = std::min(d2x, std::min(d2y, d2z));
                constexpr float distToDrag = 5.f;
                const bool canDrag = minDist < distToDrag;
                DrawLineEx(toRLVec2(p2d), toRLVec2(x2d), canDrag && minDist == d2x ? 4.f : 2.f, RED);
                DrawLineEx(toRLVec2(p2d), toRLVec2(y2d), canDrag && minDist == d2y ? 4.f : 2.f, GREEN);
                DrawLineEx(toRLVec2(p2d), toRLVec2(z2d), canDrag && minDist == d2z ? 4.f : 2.f, BLUE);
                BeginMode3D(camera);

                // TODO: move to editor state
                static bool isDragging = false;
                static int dragAxis = -1;
                static float dragAxisMua = 0.f;
                static float deltaMoved = 0.f;

                if (IsMouseButtonDown(0))
                {
                    if (canDrag && !isDragging)
                    {
                        isDragging = true;
                        deltaMoved = 0.f;
                        dragAxis = minDist == d2x ? 0 : minDist == d2y ? 1 : 2;
                        float mub;
                        line_ray_closest(tov3(pivot), tov3(axes[dragAxis]), rp, rd, dragAxisMua, mub);
                    }
                    else if (isDragging)
                    {
                        float mua, mub;
                        line_ray_closest(tov3(pivot), tov3(axes[dragAxis]), rp, rd, mua, mub);
                        vec3f axis = tov3(axes[dragAxis]) - tov3(pivot);
                        float mag = axis.mag();
                        deltaMoved += mua - dragAxisMua;
                        position += axis * (mua - dragAxisMua);
                    }
                }
                else
                {
                    if (isDragging)
                        printf("delta moved %.2f\n", deltaMoved * 0.5f);
                    isDragging = false;
                }

                if (IsKeyPressed(KEY_R))
                {
                    sel.query_comp([&](float& rotation)
                    {
                        rotation += 90.f;
                        if (rotation >= 360.f)
                            rotation -= 360.f;
                    }, COMPID(float, rotation));
                }
            }, COMPID(vec3f, position));
        }, COMPID(eecs::EntityId, selectedEntity));

        eecs::query_entities(reg, [&](eecs::EntityId, eecs::EntityId& selectedEntity, const std::string& selectedPrefab)
        {
            if (!selectedPrefab.empty())
                return;
            if (selectedEntity != eecs::invalid_eid)
                return;
            // Select stuff in world
            eecs::EntityId bestEntity = eecs::invalid_eid;
            float bestT = 1e12f;
            eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, const Model& model)
            {
                Matrix matRotation = MatrixRotate(Vector3{0.f, 1.f, 0.f}, eecs::get_comp_or(reg, eid, COMPID(float, rotation), 0.f)*DEG2RAD);
                Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

                Matrix matTransform = MatrixMultiply(matRotation, matTranslation);
                RayCollision coll = GetRayCollisionMesh(r, model.meshes[0], matTransform);
                if (!coll.hit)
                    return;
                if (coll.distance < bestT)
                {
                    bestT = coll.distance;
                    bestEntity = eid;
                }
            }, COMPID(const vec3f, position), COMPID(const Model, model));
            eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, float rotation, const vec3f& dbox_offset, const vec3f& dbox_size)
            {
                Matrix matRotation = MatrixRotate(Vector3{0.f, 1.f, 0.f}, rotation*DEG2RAD);
                Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

                Matrix matTransform = MatrixMultiply(matRotation, matTranslation);
                Matrix invTransform = MatrixInvert(matTransform);
                Ray localRay{r.position * invTransform, Vector3Rotate(r.direction, invTransform)};

                BoundingBox bbox;
                bbox.min = toRLVec3(dbox_offset - dbox_size * 0.5f);
                bbox.max = toRLVec3(dbox_offset + dbox_size * 0.5f);
                RayCollision coll = GetRayCollisionBox(localRay, bbox);
                if (!coll.hit)
                    return;
                if (coll.distance < bestT)
                {
                    bestT = coll.distance;
                    bestEntity = eid;
                }
            }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const vec3f, dbox_offset), COMPID(const vec3f, dbox_size));
            eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, const vec2i& trigger_volume)
            {
                BoundingBox box = {
                    Vector3{position.x - trigger_volume.x * 0.5f, position.y - 0.1f, position.z - trigger_volume.y * 0.5f},
                    Vector3{position.x + trigger_volume.x * 0.5f, position.y + 0.1f, position.z + trigger_volume.y * 0.5f}
                };
                RayCollision coll = GetRayCollisionBox(r, box);
                if (!coll.hit)
                    return;
                if (coll.distance < bestT)
                {
                    bestT = coll.distance;
                    bestEntity = eid;
                }
            }, COMPID(const vec3f, position), COMPID(const vec2i, trigger_volume));
            if (bestEntity != eecs::invalid_eid)
            {
                eecs::EntityWrap best = eecs::wrap_entity(reg, bestEntity);
                best.query_comps([&](const Model& model, const vec3f& position)
                {
                    const float rot = eecs::get_comp_or(reg, bestEntity, COMPID(float, rotation), 0.f);
                    BoundingBox bbox = GetModelBoundingBox(model);
                    expand_box(bbox, 0.02f);
                    draw_cube_matrix(bbox, position, rot, Color{255, 255, 0, 100});
                }, COMPID(const Model, model), COMPID(const vec3f, position));
                best.query_comps([&](const vec3f& position, float rotation, const vec3f& dbox_offset, const vec3f& dbox_size, const vec3f& dbox_color)
                {
                    BoundingBox bbox;
                    expand_box(bbox, 0.02f);
                    bbox.min = toRLVec3(dbox_offset - dbox_size * 0.5f);
                    bbox.max = toRLVec3(dbox_offset + dbox_size * 0.5f);
                    draw_cube_matrix(bbox, position, rotation, Color{uint8_t(dbox_color.x * 255), uint8_t(dbox_color.y * 255), uint8_t(dbox_color.z * 255), 100});
                }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const vec3f, dbox_offset), COMPID(const vec3f, dbox_size), COMPID(const vec3f, dbox_color));
                best.query_comps([&](const vec3f& position, const vec2i& trigger_volume)
                {
                    DrawCube(toRLVec3(position), trigger_volume.x + 0.02f, 0.2f + 0.02f, trigger_volume.y + 0.02f, Color{255, 255, 0, 150});
                }, COMPID(const vec3f, position), COMPID(const vec2i, trigger_volume));
                if (IsMouseButtonReleased(0) && !best.has(COMPID(Tag, Door)) && !best.has(COMPID(Tag, wall)))
                    selectedEntity = bestEntity;
                if (IsMouseButtonReleased(1))
                    eecs::del_entity(reg, bestEntity);
            }
        }, COMPID(eecs::EntityId, selectedEntity), COMPID(const std::string, selectedPrefab));
        eecs::query_entities(reg, [&](eecs::EntityId eid, EntTypeList selectedType, const std::string& selectedPrefab)
        {
            if (selectedPrefab.empty())
                return;
            const float floorT = (-rp.y + plFloor) / rd.y;
            const float ceilingT = (1.f - rp.y + plFloor) / rd.y;
            if (floorT > 0.f && floorT < 10.f)
            {
                const vec3f intersection = rp + rd * floorT;
                int wx = floorf(intersection.x + 0.5f);
                int wy = floorf(intersection.z + 0.5f);
                vec3f tilePos = vec3f(floorf(intersection.x + 0.5f), plFloor, floorf(intersection.z + 0.5f));
                const vec3f insideTilePos = intersection - tilePos;
                constexpr float threshold = 0.3f;
                static int dirAtPress = -1;
                if ((selectedType == E_WALLS || selectedType == E_DOORS) && (fabsf(insideTilePos.x) > threshold || fabsf(insideTilePos.z) > threshold))
                {
                    int dir = fabsf(insideTilePos.x) > threshold ? 1 : 0;
                    if (dir == 1)
                    {
                        DrawCube(castRLVec3(tilePos + vec3f(sign(insideTilePos.x) * 0.5f, 0.5f, 0)), 0.065f, 1.f, 1.f, Color{255, 255, 0, 150});
                        if (insideTilePos.x > 0.f)
                        {
                            wx++;
                            tilePos.x += 1.f;
                        }
                    }
                    else
                    {
                        DrawCube(castRLVec3(tilePos + vec3f(0, 0.5f, sign(insideTilePos.z) * 0.5f)), 1.f, 1.f, 0.065f, Color{255, 255, 0, 150});
                        if (insideTilePos.z > 0.f)
                        {
                            wy++;
                            tilePos.z += 1.f;
                        }
                    }
                    if (IsMouseButtonPressed(0))
                    {
                        dirAtPress = dir;
                    }
                    if (IsMouseButtonReleased(0))
                    {
                        delete_wall(reg, tilePos, dir);
                        delete_door(reg, wx, wy, dir);
                        if (selectedType == E_WALLS)
                        {
                            vec2f hzDir = vec2f(sinf(dir * PI / 2), cosf(dir * PI / 2));
                            vec2f pos2d = vec2f(wx - (dir ? 0.5f : 0.f), wy - (dir ? 0.f : 0.5f));
                            vec2f cam2dDir = pos2d - vec2f(camera.position.x, camera.position.z);
                            create_wall(reg, tilePos, dir, hzDir.dot(cam2dDir) < 0, selectedPrefab.c_str());
                        }
                        else if (selectedType == E_DOORS)
                            create_door(reg, wx, wy, dir, selectedPrefab.c_str());
                    }
                    if (IsMouseButtonReleased(1))
                    {
                        delete_wall(reg, tilePos, dir);
                        delete_door(reg, wx, wy, dir);
                    }
                }
                if (selectedType == E_FLOORS)
                {
                    DrawCube(castRLVec3(tilePos), 1.f, 0.05f, 1.f, Color{255, 255, 0, 150});
                    if (IsMouseButtonDown(0))
                    {
                        delete_floor(reg, tilePos);
                        create_floor(reg, tilePos, camRot, selectedPrefab.c_str());
                    }
                    if (IsMouseButtonReleased(1))
                        delete_floor(reg, tilePos);
                }
                if (selectedType == E_ENTITIES)
                {
                    DrawCube(castRLVec3(tilePos), 1.f, 0.05f, 1.f, Color{255, 255, 0, 150});
                    if (IsMouseButtonReleased(0))
                        create_entity(reg, tilePos.x, tilePos.z, camRot, selectedPrefab.c_str());
                }
                if (selectedType == E_LOGIC)
                {
                    DrawCube(castRLVec3(tilePos), 1.f, 0.05f, 1.f, Color{255, 255, 0, 150});
                    if (IsMouseButtonReleased(0))
                        create_logic(reg, tilePos.x, tilePos.z, camRot, selectedPrefab.c_str());
                }
                if (selectedType == E_COLUMNS)
                {
                    vec3f tpos = tilePos + vec3f(sign(insideTilePos.x) * 0.5f, 0.5f, sign(insideTilePos.z) * 0.5f);
                    DrawCube(castRLVec3(tpos), 0.13f, 1.f, 0.13f, Color{255, 255, 0, 150});
                    int cx = tilePos.x + (insideTilePos.x > 0 ? 1 : 0);
                    int cy = tilePos.z + (insideTilePos.z > 0 ? 1 : 0);
                    if (IsMouseButtonReleased(0))
                    {
                        delete_column(reg, cx, cy);
                        create_column(reg, cx, cy, selectedPrefab.c_str());
                    }
                    if (IsMouseButtonReleased(1))
                        delete_column(reg, cx, cy);
                }
            }
            if (ceilingT > 0.f && ceilingT < 10.f)
            {
                const vec3f intersection = rp + rd * ceilingT;
                int wx = floorf(intersection.x + 0.5f);
                int wy = floorf(intersection.z + 0.5f);
                vec3f tilePos = vec3f(floorf(intersection.x + 0.5f), 1.f, floorf(intersection.z + 0.5f));
                if (selectedType == E_CEILINGS)
                {
                    DrawCube(castRLVec3(tilePos), 1.f, 0.05f, 1.f, Color{255, 255, 0, 150});
                    if (IsMouseButtonDown(0))
                    {
                        delete_ceiling(reg, tilePos.x, tilePos.z);
                        create_ceiling(reg, tilePos.x, tilePos.z, camRot, selectedPrefab.c_str());
                    }
                    if (IsMouseButtonReleased(1))
                        delete_ceiling(reg, tilePos.x, tilePos.z);
                }
            }
        }, COMPID(const EntTypeList, selectedType), COMPID(const std::string, selectedPrefab));
    }, COMPID(const Camera, camera));
    eecs::reg_system(reg, [&](eecs::EntityId eid, const Camera& camera)
      {
          eecs::query_entities(reg, [&](eecs::EntityId, const vec3f& position, float rotation, const vec3f& dbox_offset, const vec3f& dbox_size, const vec3f& dbox_color,
                                        const std::string& dbox_name)
          {
            BoundingBox bbox;
            bbox.min = toRLVec3(dbox_offset - dbox_size * 0.5f);
            bbox.max = toRLVec3(dbox_offset + dbox_size * 0.5f);
            draw_cube_matrix(bbox, position, rotation, Color{uint8_t(dbox_color.x * 255), uint8_t(dbox_color.y * 255), uint8_t(dbox_color.z * 255), 150});
            Matrix matRotation = MatrixRotate(Vector3{0.f, 1.f, 0.f}, rotation*DEG2RAD);
            Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

            Matrix matTransform = MatrixMultiply(matRotation, matTranslation);
            Vector3 topBox = (Vector3{(bbox.min.x + bbox.max.x) * 0.5f, bbox.max.y, (bbox.min.x + bbox.max.x) * 0.5f}) * matTransform;
            Vector3 screenPos = GetWorldToScreen3dEx(topBox, camera);
            if (screenPos.z >= 0.f)
            {
              EndMode3D();
              draw_centered_font_with_shadow(GetFontDefault(), dbox_name.c_str(), torect(screenPos.x, screenPos.y, 0.f, 0.f), 12.f, WHITE);
              BeginMode3D(camera);
            }
          }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const vec3f, dbox_offset),
              COMPID(const vec3f, dbox_size), COMPID(const vec3f, dbox_color), COMPID(const std::string, dbox_name));
          eecs::query_entities(reg, [&](eecs::EntityId, const vec3f& position, const vec2i& trigger_volume, const std::string& trigger_debugName)
          {
              DrawCube(toRLVec3(position), trigger_volume.x, 0.2f, trigger_volume.y, Color{255, 255, 255, 150});
              Vector3 topBox = Vector3{position.x, position.y + 0.2f, position.z};
              Vector3 screenPos = GetWorldToScreen3dEx(topBox, camera);
              if (screenPos.z >= 0.f)
              {
                  EndMode3D();
                  draw_centered_font_with_shadow(GetFontDefault(), trigger_debugName.c_str(), torect(screenPos.x, screenPos.y, 0.f, 0.f), 12.f, WHITE);
                  BeginMode3D(camera);
              }
          }, COMPID(const vec3f, position), COMPID(const vec2i, trigger_volume), COMPID(const std::string, trigger_debugName));
      }, COMPID(const Camera, camera));

    eecs::create_entity_wrap(reg)
        .set(COMPID(EntTypeList, selectedType), E_FLOORS)
        .set(COMPID(std::string, selectedPrefab), std::string())
        .set(COMPID(eecs::EntityId, selectedEntity), eecs::invalid_eid);
}


void save_level(eecs::Registry& reg, const char* filename)
{
    std::string fullPath = "res/levels/";
    fullPath += filename;
    std::string edat = "";
    eecs::ComponentHandlers handlers;
    handlers.addTypeHandler<float>([&](const std::string_view& view, float val)
    {
        edat += std::string("    ") + std::string(view) + " : float = \"" + std::to_string(val) + "\"\n";
    });
    handlers.addTypeHandler<Tag>([&](const std::string_view& view, Tag val)
    {
        edat += std::string("    ") + std::string(view) + " : Tag = \"\"\n";
    });
    handlers.addTypeHandler<vec3f>([&](const std::string_view& view, vec3f val)
    {
        edat += std::string("    ") + std::string(view) + " : float[3] = [" +
            "\"" + std::to_string(val.x) + "\", "
            "\"" + std::to_string(val.y) + "\", "
            "\"" + std::to_string(val.z) + "\""
            "]\n";
    });
    handlers.addTypeHandler<std::string>([&](const std::string_view& view, const std::string& val)
    {
        edat += std::string("    ") + std::string(view) + " : str = \"" + val + "\"\n";
    });
#define REG_EMPTY_HANDLER(type) handlers.addTypeHandler<type>([&](const std::string_view& view, const type& val) {});
    REG_EMPTY_HANDLER(Model);
    REG_EMPTY_HANDLER(Texture2D);
    eecs::query_entities(reg, [&](eecs::EntityId eid, Tag Saveable)
    {
        edat += "_" + std::to_string(eid) + " = {\n";
        eecs::handle_entity_components(reg, eid, handlers);
        edat += "}\n";
    }, COMPID(const Tag, Saveable));
    if (!edat.empty())
        edat.resize(edat.size() - 1);
    std::ofstream file(fullPath);
    file << edat;
}

void create_floor(eecs::Registry& reg, vec3f tilePos, float rot, const char* name)
{
    eecs::create_wrap_from_prefab(reg, eecs::find_entity(reg, name))
        .tag(COMPID(Tag, Saveable))
        .set(COMPID(float, rotation), rot)
        .set(COMPID(vec3f, position), {float(tilePos.x), tilePos.y-0.025f, float(tilePos.z)});
}

void create_entity(eecs::Registry& reg, int x, int y, float rot, const char* name)
{
    eecs::EntityWrap pref = eecs::find_entity_wrap(reg, name);
    const vec3f relPos = pref.get_or(COMPID(vec3f, relativePos), vec3f(0, 0, 0));
    const Matrix matRotation = MatrixRotate(tovec3(0, 1, 0), rot * DEG2RAD);
    const vec3f tilePos = vec3f(x, 0, y);
    const vec3f pos = tilePos + tov3(castRLVec3(relPos) * matRotation);
    eecs::create_wrap_from_prefab(reg, pref)
        .tag(COMPID(Tag, Saveable))
        .set(COMPID(float, rotation), rot + 180)
        .set(COMPID(vec3f, position), pos);
}

void create_logic(eecs::Registry& reg, int x, int y, float rot, const char* name)
{
    eecs::EntityWrap pref = eecs::find_entity_wrap(reg, name);
    const vec3f tilePos = vec3f(x, 0, y);
    const vec3f pos = tilePos;
    eecs::create_wrap_from_prefab(reg, pref)
        .tag(COMPID(Tag, Saaveable))
        .set(COMPID(float, rotation), rot + 180)
        .set(COMPID(vec3f, position), pos);
}


void create_ceiling(eecs::Registry& reg, int x, int y, float rot, const char* name)
{
    eecs::create_wrap_from_prefab(reg, eecs::find_entity(reg, name))
        .tag(COMPID(Tag, Saveable))
        .set(COMPID(float, rotation), rot)
        .set(COMPID(vec3f, position), {float(x), 1.025f, float(y)});
}

void create_wall(eecs::Registry& reg, vec3f tilePos, int dir, bool flip, const char* name)
{
    vec3f pos = vec3f(tilePos.x - (dir ? 0.5f : 0.f), tilePos.y + 0.5f, tilePos.z - (dir ? 0.f : 0.5f));
    eecs::create_wrap_from_prefab(reg, eecs::find_entity(reg, name))
        .tag(COMPID(Tag, Saveable))
        .set(COMPID(float, rotation), dir * 90.f + (flip ? 180.f : 0.f))
        .set(COMPID(vec3f, position), pos);
}

void create_door(eecs::Registry& reg, int x, int y, int dir, const char* name)
{
    vec3f pos = vec3f(x - (dir ? 0.5f : 0.f), 0.5f, y - (dir ? 0.f : 0.5f));
    eecs::create_wrap_from_prefab(reg, eecs::find_entity(reg, name))
        .tag(COMPID(Tag, Door))
        .tag(COMPID(Tag, Saveable))
        .set(COMPID(float, rotation), dir * 90.f)
        .set(COMPID(vec3f, position), pos);
}


void create_column(eecs::Registry& reg, int x, int y, const char* name)
{
    eecs::create_wrap_from_prefab(reg, eecs::find_entity(reg, name))
        .tag(COMPID(Tag, Column))
        .tag(COMPID(Tag, Saveable))
        .set(COMPID(float, rotation), 0.f)
        .set(COMPID(vec3f, position), {float(x) - 0.5f, 0.5f, float(y) - 0.5f});
}

void delete_floor(eecs::Registry& reg, vec3f tilePos)
{
    eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, Tag floor)
    {
        if (int(position.x) == int(tilePos.x) && int(position.y) == int(tilePos.y) && int(position.z) == int(tilePos.z))
            eecs::del_entity(reg, eid);
    }, COMPID(const vec3f, position), COMPID(const Tag, floor));
}

void delete_ceiling(eecs::Registry& reg, int x, int y)
{
    eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, Tag ceiling)
    {
        if (int(position.x) == x && int(position.z) == y)
            eecs::del_entity(reg, eid);
    }, COMPID(const vec3f, position), COMPID(const Tag, ceiling));
}


void delete_wall(eecs::Registry& reg, vec3f tilePos, int dir)
{
    eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, float rotation, Tag wall)
    {
        if (rotation != dir * 90.f && rotation != dir * 90.f + 180.f)
            return;
        if (position.x == tilePos.x - dir * 0.5f && position.z == tilePos.z - (1 - dir) * 0.5f && position.y == tilePos.y + 0.5f)
            eecs::del_entity(reg, eid);
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, wall));
}

void delete_door(eecs::Registry& reg, int x, int y, int dir)
{
    eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, float rotation, Tag Door)
    {
        if (rotation != dir * 90.f)
            return;
        if (position.x == x - dir * 0.5f && position.z == y - (1 - dir) * 0.5f)
            eecs::del_entity(reg, eid);
    }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const Tag, Door));
}


void delete_column(eecs::Registry& reg, int x, int y)
{
    eecs::query_entities(reg, [&](eecs::EntityId eid, const vec3f& position, Tag Column)
    {
        if (position.x == x - 0.5f && position.z == y - 0.5f)
            eecs::del_entity(reg, eid);
    }, COMPID(const vec3f, position), COMPID(const Tag, Column));
}

void init_app(eecs::Registry& reg) {}

void set_last_level(const char* level)
{
}

