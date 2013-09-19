#include "game/physics/shape_rect.h"
#include <stdlib.h>


void shape_rect_free(void *data) {
    free(data);
}

void shape_rect_create(shape *shape, int w, int h) {
    shape_rect *rect = malloc(sizeof(shape_rect));
    rect->size = vec2i_create(w,h);
    shape->type = SHAPE_TYPE_RECT;
    shape->data = rect;
    shape->free = shape_rect_free;
}

vec2i shape_rect_get_size(shape *shape) {
    return ((shape_rect*)shape->data)->size;
}

void shape_rect_set_size(shape *shape, vec2i size) {
    ((shape_rect*)shape->data)->size.x = size.x;
    ((shape_rect*)shape->data)->size.y = size.y;
}