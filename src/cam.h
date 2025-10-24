#pragma once

void register_cam(eecs::Registry& reg);
eecs::EntityId create_cam(eecs::Registry& reg);

void update_cam(eecs::Registry& reg);

Vector3 GetWorldToScreen3dEx(Vector3 position, Camera camera);

