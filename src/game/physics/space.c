#include "game/physics/space.h"

physics_space *global_space = NULL;

void physics_space_init() {
    global_space = malloc(sizeof(physics_space));
    vector_init(&global_space->objects, sizeof(object*));
}

void physics_space_tick() {
    // Check for all objects that have obj->can_collide set to 1
    // If collision happens, call obj->collision_[soft|hard]
    // Soft collision = Bounding box collision, no object reaction
    // Hard collision = Objects react to each other somehow
    
    // We may need to make an intersection function for eg. box vs. box collision and such.
    // Right now the shaped themselves to the collision checks in a callback function, and
    // they only check if pixel is inside a box.
    
    // We should also make "inside-out" box, that checks that object is inside the box and
    // not outside (or something) for making sure the hars do not go outside the map.
    // Or maybe just make the walls out of rectangles ? Or something.
}

void physics_space_add(object *obj) {
    vector_append(&global_space->objects, &obj);
}

void physics_space_remove(object *obj) {
    iterator it;
    object *o;
    vector_iterator_begin(&global_space->objects, &it);
    while((o = iter_next(&it)) != NULL) {
        if(*o == obj) {
            vector_delete(&it);
        }
    }
}

void physics_space_close() {
    vector_free(&global_space->objects);
    free(global_space);
    global_space = NULL;
}
