#include "game/physics/object.h"
#include <stdlib.h>

void object_create(object *obj, int px, int py, float vx, float vy, int can_collide) {
    obj->pos.x = px;
    obj->pos.y = py;
    obj->vel.x = vx;
    obj->vel.y = vy;
    obj->can_collide = can_collide;
    obj->userdata = NULL;
    obj->shape = NULL;
    obj->collision_soft = NULL;
    obj->collision_hard = NULL;
}

void object_free(object *obj) {
    if(obj->shape != NULL) {
        free(obj->shape);
    }
    if(obj->userdata != NULL) {
        free(obj->userdata);
    }
}
