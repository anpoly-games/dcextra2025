#include <raylib.h>
#include <eecs.h>
#include "debug.h"
#include "raymath.h"
#include "dcengine/tags.h"
#include "dcengine/math.h"
#include "dcengine/cam.h"
#include "dcengine/ui.h"

void register_debug(eecs::Registry& reg)
{
    eecs::reg_system(reg, [&](eecs::EntityId eid, const Camera& camera)
    {
        // draw trigger boxes
        eecs::query_entities(reg, [&](eecs::EntityId, const vec3f& position, const vec2i& trigger_volume, const std::string& trigger_debugName)
        {
            DrawCube(toRLVec3(position), trigger_volume.x, 0.2f, trigger_volume.y, Color{255, 255, 255, 150});
            Vector3 topBox = Vector3{position.x, position.y + 0.2f, position.z};
            Vector3 screenPos = GetWorldToScreen3dEx(reg, topBox, camera);
            if (screenPos.z >= 0.f)
            {
                EndMode3D();
                draw_centered_font_with_shadow(GetFontDefault(), trigger_debugName.c_str(), torect(screenPos.x, screenPos.y, 0.f, 0.f), 12.f, 3, WHITE);
                BeginMode3D(camera);
            }
        }, COMPID(const vec3f, position), COMPID(const vec2i, trigger_volume), COMPID(const std::string, trigger_debugName));

        // draw spawn points or any entity with dboxes
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
            Vector3 screenPos = GetWorldToScreen3dEx(reg, topBox, camera);
            if (screenPos.z >= 0.f)
            {
              EndMode3D();
              draw_centered_font_with_shadow(GetFontDefault(), dbox_name.c_str(), torect(screenPos.x, screenPos.y, 0.f, 0.f), 12.f, 3, WHITE);
              BeginMode3D(camera);
            }
          }, COMPID(const vec3f, position), COMPID(const float, rotation), COMPID(const vec3f, dbox_offset),
              COMPID(const vec3f, dbox_size), COMPID(const vec3f, dbox_color), COMPID(const std::string, dbox_name));
    }, COMPID(const Camera, camera));
}
