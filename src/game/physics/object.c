#include "game/physics/object.h"
#include <stdlib.h>

void object_create(object *obj, int px, int py, float vx, float vy) {
    obj->pos.x = px;
    obj->pos.y = py;
    obj->vel.x = vx;
    obj->vel.y = vy;
    obj->is_static = 0;
    obj->layers = OBJECT_DEFAULT_LAYER;
    obj->group = OBJECT_NO_GROUP;
    obj->userdata = NULL;
    obj->col_shape_hard = NULL;
    obj->col_shape_soft = NULL;
    obj->ev_collision = NULL;
}

void object_free(object *obj) {
    if(obj->col_shape_soft != NULL) {
        shape_free(obj->col_shape_soft);
        free(obj->col_shape_soft);
        obj->col_shape_soft = NULL;
    }
    if(obj->col_shape_hard != NULL) { 
        shape_free(obj->col_shape_hard);
        free(obj->col_shape_hard);
        obj->col_shape_hard = NULL;
    }
}

void object_ev_cb_register(object *obj, ev_collision_callback cb) {
    obj->ev_collision = cb;
}

void object_set_layers(object *obj, int layers) {
    obj->layers = layers;
}

void object_set_group(object *obj, int group) {
    obj->group = group;
}

void object_set_gravity(object *obj, float gravity) {
    obj->gravity = gravity;
}

void object_set_hard_shape(object *obj, shape *sh) {
    obj->col_shape_hard = sh;
}

void object_set_soft_shape(object *obj, shape *sh) {
    obj->col_shape_soft = sh;
}

void object_set_static(object *obj, int is_static) {
    obj->is_static = is_static;
}

int  object_get_px(object *obj) { return obj->pos.x; }
int  object_get_py(object *obj) { return obj->pos.y; }
void object_set_px(object *obj, int px) { obj->pos.x = px; }
void object_set_py(object *obj, int py) { obj->pos.y = py; }

float object_get_vx(object *obj) { return obj->vel.x; }
float object_get_vy(object *obj) { return obj->vel.y; }
void  object_set_vx(object *obj, float vx) { obj->vel.x = vx; }
void  object_set_vy(object *obj, float vy) { obj->vel.y = vy; }

void object_get_pos(object *obj, int *px, int *py) {
    *px = obj->pos.x;
    *py = obj->pos.y;
}

void object_set_pos(object *obj, int px, int py) {
    obj->pos.x = px;
    obj->pos.y = py;
}

void object_add_pos(object *obj, int px, int py) {
    obj->pos.x += px;
    obj->pos.y += py;
}

void object_get_vel(object *obj, float *vx, float *vy) {
    *vx = obj->vel.x;
    *vy = obj->vel.y;
}

void object_set_vel(object *obj, float vx, float vy) {
    obj->vel.x = vx;
    obj->vel.y = vy;
}

void object_add_vel(object *obj, float vx, float vy) {
    obj->vel.x += vx;
    obj->vel.y += vy;
}
