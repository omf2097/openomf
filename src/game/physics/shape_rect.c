#include "game/physics/shape_rect.h"
#include <stdlib.h>

int shape_rect_check_collision(void *data, vec2f check_pos, vec2f self_pos) {
    shape_rect *rect = (shape_rect*)data;
    if(check_pos.x > self_pos.x && check_pos.x < self_pos.x + rect->size.x &&
       check_pos.y > self_pos.y && check_pos.y < self_pos.y + rect->size.y) {
        return 1;
    }
    return 0;
}

void shape_rect_free(void *data) {
    free(data);
}

void shape_rect_create(shape *shape, int w, int h) {
    shape_rect *rect = malloc(sizeof(shape_rect));
    rect->size.x = w;
    rect->size.y = h;
    shape->data = rect;
    shape->check_collision = shape_rect_check_collision;
    shape->free = shape_rect_free;
}
