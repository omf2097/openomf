#include "game/physics/space.h"

cpSpace *global_space = NULL;

void physics_space_init() {
    global_space = cpSpaceNew();
}

void physics_space_set_gravity(cpFloat gravity) {
    cpSpaceSetGravity(global_space, cpv(0, gravity));
}

void physics_space_tick(float time) {
    cpSpaceStep(global_space, time);
}

void physics_space_close() {
    cpSpaceFree(global_space);
}
