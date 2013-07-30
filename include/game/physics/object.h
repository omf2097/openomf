#ifndef _OBJECT_H
#define _OBJECT_H

#include <chipmunk/chipmunk.h>

typedef struct object_t {
    cpSpace *space;
    cpBody *body;
    cpShape *shape;
    
    cpFloat friction;
    cpFloat elasticity;
} object;

void object_create(object *obj, cpSpace *space, cpFloat px, cpFloat py, cpFloat vx, cpFloat vy, cpFloat mass, cpFloat friction, cpFloat elasticity);
void object_set_collision_box(object *obj, int w, int h);
void object_set_friction(object *obj, cpFloat friction);
void object_set_elasticity(object *obj, cpFloat elasticity);
void object_set_vel(object *obj, cpFloat vx, cpFloat vy);
void object_set_pos(object *obj, int px, int py);
void object_set_px(object *obj, int px);
void object_set_py(object *obj, int py);
void object_add_vel(object *obj, cpFloat vx, cpFloat vy);
void object_add_pos(object *obj, int px, int py);
void object_get_vel(object *obj, cpFloat *vx, cpFloat *vy);
void object_get_pos(object *obj, int *px, int *py);
int object_get_px(object *obj); // Position X
int object_get_py(object *obj); // Position Y
void object_set_group(object *obj, unsigned int group);
void object_set_gravity(object *obj, cpFloat gravity);
void object_free(object *obj);

#endif // _OBJECT_H