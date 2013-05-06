#include "game/physics/physics.h"

void physics_init(physics_state *state, int pos_x, int pos_y, float spd_x, float spd_y) {
    state->pos.x = pos_x;
    state->pos.y = pos_y;
    state->spd.x = spd_x;
    state->spd.y = spd_y;
}

/*
 * As long as Y speed is larger than 0, we are flying or jumping.
 * Keep X speed as static, until we hit ground again.
 * In Y, take gravity into account.
 */
void physics_tick(physics_state *state) {
    if(state->spd.y > 0) {
        state->pos.y += state->spd.y;
        state->spd.y -= PHYSICS_G;
        if(state->pos.y <= 0) {
            state->spd.y = 0;
            state->spd.x = 0;
        }
    }
    if(state->spd.x > 0) {
        state->pos.x += state->spd.x;
    }
}
