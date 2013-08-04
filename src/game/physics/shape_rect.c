#include "game/physics/shape_rect.h"
#include <stdlib.h>


void shape_rect_free(void *data) {
    free(data);
}

void shape_rect_create(shape *shape, int w, int h) {
    shape_rect *rect = malloc(sizeof(shape_rect));
    rect->size.x = w;
    rect->size.y = h;
    shape->type = SHAPE_TYPE_RECT;
    shape->data = rect;
    shape->free = shape_rect_free;
}

vec2i shape_rect_get_size(shape *shape) {
    return ((shape_rect*)shape->data)->size;
}
