#ifndef _SHAPE_RECT_H
#define _SHAPE_RECT_H

#include "game/physics/shape.h"
#include "utils/vec.h"

typedef struct shape_rect_t {
    vec2i size;
} shape_rect;

void shape_rect_create(shape *shape, int w, int h);
vec2i shape_rect_get_size(shape *shape);
void shape_rect_set_size(shape *shape, vec2i size);

#endif // _SHAPE_RECT_H