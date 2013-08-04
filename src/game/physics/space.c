#include "game/physics/space.h"
#include <stdlib.h>

physics_space *global_space = NULL;

void physics_space_init() {
    global_space = malloc(sizeof(physics_space));
    vector_create(&global_space->objects, sizeof(object*));
}

void physics_space_tick() {
    // Collisions:
    // - Happen only for objects that are marked to be on same layers (bitmask obj->layers)
    // - Happen only for objects that are in different groups (objects in same group do not interact)
    
    // If collision happens:
    // Soft collision = Bounding box collision, call user defined callback (if not NULL)
    // Hard collision = Stop object or bounce it off (depending on what has been defined)
    
    // Intersections can be checked with shape_intersect. 
    
    // We should also make "inside-out" box, that checks that object is inside the box and
    // not outside (or something) for making sure the hars do not go outside the map.
    // Or maybe just make the walls out of rectangles ? Or something.
}

void physics_space_add(object *obj) {
    vector_append(&global_space->objects, &obj);
}

void physics_space_remove(object *obj) {
    iterator it;
    object **o;
    vector_iter_begin(&global_space->objects, &it);
    while((o = iter_next(&it)) != NULL) {
        if(*o == obj) {
            vector_delete(&global_space->objects, &it);
        }
    }
}

void physics_space_close() {
    vector_free(&global_space->objects);
    free(global_space);
    global_space = NULL;
}
