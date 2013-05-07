#include "game/physics/physics.h"

void physics_init(physics_state *state, int pos_x, int pos_y, float spd_x, float spd_y) {
    state->pos.x = pos_x;
    state->pos.y = pos_y;
    state->spd.x = spd_x;
    state->spd.y = spd_y;
    state->in_air = 0;
}

/*
 * As long as Y speed is larger than 0, we are flying or jumping.
 * Keep X speed as static, until we hit ground again.
 * In Y, take gravity into account.
 */
void physics_tick(physics_state *state) {
    if(state->in_air) {
        state->pos.y += state->spd.y;
        state->spd.y -= PHYSICS_G;
        if(state->pos.y <= 0) {
            state->spd.y = 0;
            state->spd.x = 0;
            state->in_air = 0;
        }
    }
    if(state->spd.x > 0) {
        state->pos.x += state->spd.x;
    }
}

/*
 * Allow jumping only if HAR is not already in the air
 */
void physics_jump(physics_state *state, int spd_x, int spd_y)  {
    if(!state->in_air) {
        state->spd.x = spd_x;
        state->spd.y = spd_y;
        state->in_air = 1;
    }
}

/*
 * Allow HAR getting kicked in the air
 */
void physics_recoil(physics_state *state, int spd_x, int spd_y) {
    state->spd.x = spd_x;
    state->spd.y = spd_y;
    state->in_air = 1;
}


void physics_move(physics_state *state, int spd_x) {
    if(!state->in_air) {
        state->spd.x = spd_x;
    }
}

int physics_is_falling(physics_state *state) {
    return (state->spd.y < 0);
}

int physics_is_stopped(physics_state *state) {
    return (state->spd.x == 0 && state->spd.y == 0);
}