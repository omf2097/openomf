#ifndef _SPACE_H
#define _SPACE_H

#include <chipmunk/chipmunk.h>

extern cpSpace *global_space;

void physics_space_init();
void physics_space_set_gravity(cpFloat gravity);
void physics_space_tick();
void physics_space_close();

#endif // _SPACE_H