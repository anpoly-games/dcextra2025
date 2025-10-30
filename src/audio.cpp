#include <raylib.h>
#include <eecs.h>
#include <string>

#include "audio.h"

void register_audio(eecs::Registry& reg)
{
    eecs::reg_enter(reg, [&](eecs::EntityId eid, const std::string& sound_filename)
    {
        Sound sound = LoadSound(sound_filename.c_str());
        eecs::set_component(reg, eid, COMPID(Sound, sound), sound);
    }, COMPID(const std::string, sound_filename));
}

