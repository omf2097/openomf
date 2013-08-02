#include "game/physics/space.h"

physics_space *global_space = NULL;

void physics_space_init() {

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

}

void physics_space_remove(object *obj) {

}

void physics_space_close() {

}
