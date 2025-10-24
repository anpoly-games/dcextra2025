#pragma once

void register_audio(eecs::Registry& reg);

//void play_snd(flecs::entity e);
//void stop_snd(flecs::entity e);

struct StepSound {};
struct DoorSound {};
struct ClickSound {};
struct TriggerSound {};
struct LockedSound {};
struct DialogSound {};

struct ShootSound {};
struct HitSound {};

