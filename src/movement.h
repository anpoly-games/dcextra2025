#pragma once

#include "math.h"

struct Position : public vec3f { ASSIGN_OP_3f(Position); };
struct Direction : public vec3f { ASSIGN_OP_3f(Direction); };
struct Rotation { float val = 0.f; };

