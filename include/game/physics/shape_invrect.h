#ifndef _SHAPE_INVRECT_H
#define _SHAPE_INVRECT_H

#include "game/physics/shape.h"
#include "utils/vec.h"

typedef struct shape_invrect_t {
    vec2i size;
} shape_invrect;

void shape_invrect_create(shape *shape, int w, int h);
vec2i shape_invrect_get_size(shape *shape);

#endif // _SHAPE_INVRECT_H