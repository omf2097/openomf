#ifndef _INTERSECT_H
#define _INTERSECT_H

#include "game/physics/shape.h"

int shape_intersect(shape *a, vec2i ca, shape *b, vec2i cb);

#endif // _INTERSECT_H