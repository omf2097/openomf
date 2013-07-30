#include "game//physics/object.h"
#include <stdlib.h>

typedef struct object_userdata_t {
    cpFloat gravity;
} object_userdata;

void object_create(object *obj, cpSpace *space, cpFloat px, cpFloat py, cpFloat vx, cpFloat vy, cpFloat mass, cpFloat friction, cpFloat elasticity) {
    obj->space = space;
    obj->friction = friction;
    obj->elasticity = elasticity;
    obj->body = cpSpaceAddBody(obj->space, cpBodyNew(mass, INFINITY));
    obj->body->data = malloc(sizeof(object_userdata));
    cpBodySetPos(obj->body, cpv(px, py));
    cpBodySetVel(obj->body, cpv(vx, vy));
    obj->shape = cpSpaceAddShape(obj->space, cpCircleShapeNew(obj->body, 5.0f, cpvzero));
    cpShapeSetFriction(obj->shape, friction);
    cpShapeSetElasticity(obj->shape, elasticity);
}

void object_set_collision_box(object *obj, int w, int h) {
    cpSpaceRemoveShape(obj->space, obj->shape);
    cpShapeFree(obj->shape);
    obj->shape = cpSpaceAddShape(obj->space, cpBoxShapeNew(obj->body, w, h));
    cpShapeSetFriction(obj->shape, obj->friction);
    cpShapeSetElasticity(obj->shape, obj->elasticity);
}

void object_set_friction(object *obj, cpFloat friction) {
    cpShapeSetFriction(obj->shape, friction);
}

void object_set_elasticity(object *obj, cpFloat elasticity) {
    cpShapeSetElasticity(obj->shape, elasticity);
}

void object_set_vel(object *obj, cpFloat vx, cpFloat vy) {
    cpBodySetVel(obj->body, cpv(vx, vy));
}

void object_set_pos(object *obj, int px, int py) {
    cpBodySetPos(obj->body, cpv(px, py));
}

void object_set_px(object *obj, int px) {
    cpVect npos = cpBodyGetPos(obj->body);
    cpBodySetPos(obj->body, cpv(px, npos.y));
}

void object_set_py(object *obj, int py) {
    cpVect npos = cpBodyGetPos(obj->body);
    cpBodySetPos(obj->body, cpv(npos.x, py));
}

void object_get_vel(object *obj, cpFloat *vx, cpFloat *vy) {
    cpVect nvel = cpBodyGetVel(obj->body);
    *vx = nvel.x;
    *vy = nvel.y;
}

void object_get_pos(object *obj, int *px, int *py) {
    cpVect npos = cpBodyGetPos(obj->body);
    *px = npos.x;
    *py = npos.y;
}

int object_get_px(object *obj) {
    return cpBodyGetPos(obj->body).x;
}

int object_get_py(object *obj) {
    return cpBodyGetPos(obj->body).y;
}

void object_add_vel(object *obj, cpFloat vx, cpFloat vy) {
    cpVect nvel = cpBodyGetVel(obj->body);
    nvel.x += vx;
    nvel.y += vy;
    cpBodySetVel(obj->body, nvel);
}

void object_add_pos(object *obj, int px, int py) {
    cpVect npos = cpBodyGetPos(obj->body);
    npos.x += px;
    npos.y += py;
    cpBodySetPos(obj->body, npos);
}

// TODO: This function is atm. complete guesswork. Do something about it.
static void no_grav_vel_func(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
    cpFloat grav = ((object_userdata*)body->data)->gravity;
    cpBodyUpdateVelocity(body, cpv(0, grav), damping, dt);
}

void object_set_gravity(object *obj, cpFloat gravity) {
    ((object_userdata*)obj->body->data)->gravity = gravity;
    obj->body->velocity_func = no_grav_vel_func;
}

void object_set_group(object *obj, unsigned int group) {
    cpShapeSetGroup(obj->shape, group);
}

void object_free(object *obj) {
    cpSpaceRemoveShape(obj->space, obj->shape);
    cpSpaceRemoveBody(obj->space, obj->body);
    free((object_userdata*)obj->body->data);
    cpShapeFree(obj->shape);
    cpBodyFree(obj->body);
}
