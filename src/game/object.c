#include "game/object.h"

void object_create(object *obj, cpSpace *space, cpFloat px, cpFloat py, cpFloat vx, cpFloat vy, cpFloat mass, cpFloat friction, cpFloat elasticity) {
    obj->space = space;
    obj->body = cpSpaceAddBody(obj->space, cpBodyNew(mass, INFINITY));
    cpBodySetPos(obj->body, cpv(px, py));
    cpBodySetVel(obj->body, cpv(vx, vy));
    obj->shape = cpSpaceAddShape(obj->space, cpCircleShapeNew(obj->body, 5.0f, cpvzero));
    cpShapeSetFriction(obj->shape, friction);
    cpShapeSetElasticity(obj->shape, elasticity);
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

static void no_grav_vel_func(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
    body->v = cpvmult(body->f, body->m_inv);
}

void object_enable_gravity(object *obj, int enabled) {
    if(enabled) {
        obj->body->velocity_func = cpBodyUpdateVelocity;
    } else {
        obj->body->velocity_func = no_grav_vel_func;
    }
}

void object_set_group(object *obj, unsigned int group) {
    cpShapeSetGroup(obj->shape, group);
}

void object_free(object *obj) {
    cpSpaceRemoveShape(obj->space, obj->shape);
    cpSpaceRemoveBody(obj->space, obj->body);
    cpShapeFree(obj->shape);
    cpBodyFree(obj->body);
}
