#include "game/physics/shape_invrect.h"
#include <stdlib.h>

void shape_invrect_free(void *data) {
    free(data);
}

void shape_invrect_create(shape *shape, int w, int h) {
    shape_invrect *irect = malloc(sizeof(shape_invrect));
    irect->size.x = w;
    irect->size.y = h;
    shape->type = SHAPE_TYPE_INVRECT;
    shape->data = irect;
    shape->free = shape_invrect_free;
}

vec2i shape_invrect_get_size(shape *shape) {
    return ((shape_invrect*)shape->data)->size;
}
