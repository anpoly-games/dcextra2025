#include <eecs.h>
#include <raylib.h>

#include "../math.h"
#include "../cam.h"
#include "../tags.h"

eecs::EntityId create_cam(eecs::Registry& reg)
{
    const char* prefabName = "PlayerCam";
    eecs::EntityWrap cam = eecs::create_wrap_from_prefab(reg, eecs::find_entity(reg, prefabName));
    float normFov = eecs::get_comp_or(reg, cam.eid, COMPID(float, cam_normalAngle), 75.f);
    Camera camera = { { 5.0f, 5.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, normFov, 0 };
    eecs::query_entities(reg, [&](eecs::EntityId, const vec3f& position, const vec3f& direction, Tag player)
    {
        camera.position = toRLVec3(position + vec3f(0, 0.4f, 0) - direction * 0.3f);
        camera.target = toRLVec3(position + vec3f(0, 0.4f, 0) + direction);
        camera.up = Vector3{0.f, 1.f, 0.f};
    }, COMPID(const vec3f, position), COMPID(const vec3f, direction), COMPID(const Tag, player));
    return cam
        .set(COMPID(Camera, camera), camera)
        .set(COMPID(vec3f, position), {0.f, 0.4f, 0.f})
        .set(COMPID(vec3f, direction), {0.f, 0.f, 1.f}).eid;
}


