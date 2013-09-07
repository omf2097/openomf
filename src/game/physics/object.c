#include "game/physics/object.h"
#include <stdlib.h>

#include "utils/log.h"

void object_create(object *obj, int px, int py, float vx, float vy) {
    obj->pos.x = px;
    obj->pos.y = py;
    obj->vel.x = vx;
    obj->vel.y = vy;
    object_reset_vstate(obj);
    object_reset_hstate(obj);
    DEBUG("Object created. pos = (%f, %f), vel = (%f, %f)", obj->pos.x, obj->pos.y, obj->vel.x, obj->vel.y);
    obj->is_static = 0;
    obj->layers = OBJECT_DEFAULT_LAYER;
    obj->group = OBJECT_NO_GROUP;
    obj->userdata = NULL;
    obj->col_shape = NULL;
    obj->ev_collision = NULL;
    obj->gravity = 0.0f;
}

void object_free(object *obj) {
    if(obj->col_shape != NULL) {
        shape_free(obj->col_shape);
        free(obj->col_shape);
        obj->col_shape = NULL;
    }
}

void object_set_userdata(object *obj, void *ptr) {
    obj->userdata = ptr;
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

void object_set_shape(object *obj, shape *shape) {
    obj->col_shape = shape;
}

void object_set_static(object *obj, int is_static) {
    obj->is_static = is_static;
}

shape* object_get_shape(object *obj) {
    return obj->col_shape;
}

int object_is_static(object *obj) {
    return obj->is_static;
}

int object_get_gravity(object *obj) {
    return obj->gravity;
}

int object_get_group(object *obj) {
    return obj->group;
}

int object_get_layers(object *obj) {
    return obj->layers;
}

void object_reset_vstate(object *obj) {
    obj->hstate = (obj->vel.x < 0.01f && obj->vel.x > -0.01f) ? OBJECT_STABLE : OBJECT_MOVING;
}
void object_reset_hstate(object *obj) {
    obj->vstate = (obj->vel.y < 0.01f && obj->vel.y > -0.01f) ? OBJECT_STABLE : OBJECT_MOVING;
}

int  object_get_px(object *obj) { return obj->pos.x; }
int  object_get_py(object *obj) { return obj->pos.y; }
void object_set_px(object *obj, int px) { obj->pos.x = px; }
void object_set_py(object *obj, int py) { obj->pos.y = py; }

float object_get_vx(object *obj) { return obj->vel.x; }
float object_get_vy(object *obj) { return obj->vel.y; }
void  object_set_vx(object *obj, float vx) { obj->vel.x = vx; object_reset_hstate(obj); }
void  object_set_vy(object *obj, float vy) { obj->vel.y = vy; object_reset_vstate(obj); }

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
    object_reset_vstate(obj);
    object_reset_hstate(obj);
}

void object_add_vel(object *obj, float vx, float vy) {
    obj->vel.x += vx;
    obj->vel.y += vy;
    object_reset_vstate(obj);
    object_reset_hstate(obj);
}
