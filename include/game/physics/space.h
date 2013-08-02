#ifndef _SPACE_H
#define _SPACE_H

#include "game/physics/object.h"
#include "utils/vector.h"

typedef struct physics_space_t {
    vector objects;
} physics_space;

extern physics_space *global_space;

void physics_space_init();
void physics_space_tick();
void physics_space_add(object *obj);
void physics_space_remove(object *obj);
void physics_space_close();

#endif // _SPACE_H