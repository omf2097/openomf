#ifndef _OBJECT_H
#define _OBJECT_H

#include "utils/vec.h"

typedef struct object_t {
    vec2f pos;
    vec2f vel;
    int can_collide;
    shape *col_shape_hard;
    shape *col_shape_soft;
    
    void *userdata;
    void (*collision_hard)(const object *a, const object *b, void *userdata);
    void (*collision_soft)(const object *a, const object *b, void *userdata);
} object;

void object_create(object *obj, int px, int py, float vx, float vy);
void object_free(object *obj);

#endif // _OBJECT_H
