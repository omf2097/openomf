#include "game/physics/shape.h"

int shape_check_collision(void *shape, vec2f check_pos, vec2f self_pos) {
    return shape->check_collision(shape->data, check_pos, self_pos);
}

void shape_free(void *shape) {
    shape->free(shape->data);
}