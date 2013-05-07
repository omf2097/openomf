#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "utils/vec.h"

#define PHYSICS_G 1.0f

typedef struct physics_state_t physics_state;

struct physics_state_t {
    vec2i pos;
    vec2f spd;
    int in_air;
};

void physics_init(physics_state *state, int pos_x, int pos_y, float spd_x, float spd_y);
void physics_tick(physics_state *state);
void physics_jump(physics_state *state, int spd_x, int spd_y);
void physics_recoil(physics_state *state, int spd_x, int spd_y);
void physics_move(physics_state *state, int spd_x);
int physics_is_falling(physics_state *state);
int physics_is_stopped(physics_state *state);

#endif // _PHYSICS_H