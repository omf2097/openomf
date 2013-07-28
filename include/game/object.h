#ifndef _OBJECT_H
#define _OBJECT_H

#include <chipmunk/chipmunk.h>

typedef struct object_t {
    cpSpace *space;
    cpBody *body;
    cpShape *shape;
} object;

void object_create(object *obj, cpSpace *space, cpFloat px, cpFloat py, cpFloat vx, cpFloat vy, cpFloat mass, cpFloat friction, cpFloat elasticity);
void object_set_friction(object *obj, cpFloat friction);
void object_set_elasticity(object *obj, cpFloat elasticity);
void object_set_vel(object *obj, cpFloat vx, cpFloat vy);
void object_set_pos(object *obj, int px, int py);
void object_add_vel(object *obj, cpFloat vx, cpFloat vy);
void object_add_pos(object *obj, int px, int py);
void object_get_vel(object *obj, cpFloat *vx, cpFloat *vy);
void object_get_pos(object *obj, int *px, int *py);
void object_set_group(object *obj, unsigned int group);
void object_enable_gravity(object *obj, int enabled);
void object_free(object *obj);

#endif // _OBJECT_H