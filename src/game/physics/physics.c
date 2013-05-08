#include "game/physics/physics.h"
#include <stdlib.h>

#define SPD_X_ZERO(x) (x < 0.1 && x > -0.1)
#define SPD_Y_ZERO(y) (y < 0.1 && y > -0.1)

void physics_init(physics_state *state, int pos_x, int pos_y, float spd_x, float spd_y, int floor, int ceiling, int wall_left, int wall_right, float gravity, void *userdata) {
    state->pos.x = pos_x;
    state->pos.y = pos_y;
    state->spd.x = spd_x;
    state->spd.y = spd_y;
    state->wall_left = wall_left;
    state->wall_right = wall_right;
    state->ceiling = ceiling;
    state->floor = floor;
    state->gravity = gravity;
    state->userdata = userdata;
    state->flight_mode = PHY_FLIGHT_NONE;
    
    // Callbacks
    state->jump = NULL;
    state->fall = NULL;
    state->move = NULL;
    state->stop = NULL;
    state->recoil = NULL;
    state->wall_hit = NULL;
    state->floor_hit = NULL;
}

void physics_check_bounds(physics_state *state) {
    // Left/Right wall
    if(state->pos.x < state->wall_left) {
        state->pos.x = state->wall_left;
        if(state->wall_hit != NULL) {
            state->wall_hit(state, state->userdata, PHY_WALL_LEFT, state->flight_mode);
        }
    }
    else if(state->pos.x > state->wall_right) {
        state->pos.x = state->wall_right;
        if(state->wall_hit != NULL) {
            state->wall_hit(state, state->userdata, PHY_WALL_RIGHT, state->flight_mode);
        }
    }
    
    // Floor/ceiling
    if(state->pos.y <= state->ceiling) {
        state->pos.y = state->ceiling;
    }
    else if(state->pos.y >= state->floor) {
        state->pos.y = state->floor;
        if(state->flight_mode != PHY_FLIGHT_NONE) {
            if(state->floor_hit != NULL) {
                state->floor_hit(state, state->userdata, state->flight_mode);
            }
            state->flight_mode = PHY_FLIGHT_NONE;
        }
    }
}

void physics_tick(physics_state *state) {
    // Handle speed
    state->pos.y += state->spd.y;
    state->pos.x += state->spd.x;
    
    // Check bounds
    physics_check_bounds(state);
    
    // If we are flying, handle gravity
    if(state->pos.y < state->floor) {
        state->spd.y += state->gravity;
    }
}

/*
 * Allow jumping only if HAR is not already in the air
 */
void physics_jump(physics_state *state, float spd_y)  {
    if(state->flight_mode == PHY_FLIGHT_NONE) {
        state->spd.y = spd_y;
        state->flight_mode = PHY_FLIGHT_JUMP;
        if(state->jump != NULL) {
            state->jump(state, state->userdata);
        }
    }
}

/*
 * Allow HAR getting kicked in the air
 */
void physics_recoil(physics_state *state, float spd_x, float spd_y) {
    state->spd.x = spd_x;
    state->spd.y = spd_y;
    state->flight_mode = PHY_FLIGHT_RECOIL;
    if(state->recoil != NULL) {
        state->recoil(state, state->userdata);
    }
}

void physics_move(physics_state *state, float spd_x) {
    if(state->pos.y == state->floor) {
        if(state->move != NULL && !SPD_X_ZERO(spd_x) && SPD_X_ZERO(state->spd.x)){
            state->spd.x = spd_x;
            state->move(state, state->userdata);
        }
        if(state->stop != NULL && SPD_X_ZERO(spd_x) && !SPD_X_ZERO(state->spd.x)){
            state->spd.x = spd_x;
            state->stop(state, state->userdata);
        }
        state->spd.x = spd_x;
    }
}

int physics_is_stopped(physics_state *state) {
    return (state->spd.x == 0 && state->spd.y == 0);
}

int physics_is_moving_up(physics_state *state) {
    return (state->spd.y > 0);
}

int physics_is_moving_down(physics_state *state) {
    return (state->spd.y < 0);
}
