#include "game/physics/shape.h"

void shape_free(shape *shape) {
    shape->free(shape->data);
}