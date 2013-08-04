#ifndef _OBJECT_H
#define _OBJECT_H

#include "utils/vec.h"
#include "game/physics/shape.h"

#define OBJECT_NO_LAYERS 0
#define OBJECT_NO_GROUP -1

typedef struct object_t object;
typedef void (*ev_collision_callback)(const object *a, const object *b, void *userdata_a, void *userdata_b);

struct object_t {
    vec2f pos;
    vec2f vel;
    shape *col_shape_hard;
    shape *col_shape_soft;
    float gravity;
    
    int group;
    int layers;
    
    void *userdata;
    ev_collision_callback ev_collision;
};

void object_create(object *obj, int px, int py, float vx, float vy);
void object_free(object *obj);

void object_ev_cb_register(object *obj, ev_collision_callback cb);
void object_set_layers(object *obj, int layers);
void object_set_group(object *obj, int group);
void object_set_gravity(object *obj, float gravity);
void object_set_hard_shape(object *obj, shape *sh);
void object_set_soft_shape(object *obj, shape *sh);

int  object_get_px(object *obj);
int  object_get_py(object *obj);
void object_set_px(object *obj, int px);
void object_set_py(object *obj, int py);

float object_get_vx(object *obj);
float object_get_vy(object *obj);
void  object_set_vx(object *obj, float vx);
void  object_set_vy(object *obj, float vy);

void object_get_pos(object *obj, int *px, int *py);
void object_set_pos(object *obj, int px, int py);
void object_add_pos(object *obj, int px, int py);

void object_get_vel(object *obj, float *vx, float *vy);
void object_set_vel(object *obj, float vx, float vy);
void object_add_vel(object *obj, float vx, float vy);

#endif // _OBJECT_H
