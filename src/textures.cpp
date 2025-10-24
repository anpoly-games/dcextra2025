#include <eecs.h>
#include <raylib.h>

#include "textures.h"

void register_textures(eecs::Registry& reg)
{
    eecs::reg_enter(reg, [&](eecs::EntityId eid, const std::string& texture_filename)
    {
        Texture2D tex = LoadTexture(texture_filename.c_str());
        eecs::EntityWrap ent = eecs::wrap_entity(reg, eid);
        ent.set(COMPID(Texture2D, texture_diff), tex);
    }, COMPID(const std::string, texture_filename));
    eecs::reg_enter(reg, [&](eecs::EntityId eid, const std::string& texture_emissiveFilename)
    {
        Texture2D tex = LoadTexture(texture_emissiveFilename.c_str());
        eecs::EntityWrap ent = eecs::wrap_entity(reg, eid);
        ent.set(COMPID(Texture2D, texture_emissive), tex);
    }, COMPID(const std::string, texture_emissiveFilename));
}

