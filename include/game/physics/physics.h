#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "utils/vec.h"

typedef struct physics_state_t physics_state;

#define PHY_VSTATE_CROUCH 0
#define PHY_VSTATE_NONE 1
#define PHY_VSTATE_JUMP 2
#define PHY_VSTATE_RECOIL 3

#define PHY_WALL_LEFT 0
#define PHY_WALL_RIGHT 1

struct physics_state_t {
    vec2i pos;
    vec2f spd;
    int vertical_state;
    
    void *userdata;
    
    int floor;
    int ceiling;
    int wall_left;
    int wall_right;
    float gravity;
    
    void (*jump)(physics_state *state, void *userdata);
    void (*fall)(physics_state *state, void *userdata);
    void (*move)(physics_state *state, void *userdata);
    void (*stop)(physics_state *state, void *userdata);
    void (*recoil)(physics_state *state, void *userdata);
    void (*wall_hit)(physics_state *state, void *userdata, int side, int vstate);
    void (*floor_hit)(physics_state *state, void *userdata, int vstate);
    void (*crouch)(physics_state *state, void *userdata);
};

void physics_init(physics_state *state, int pos_x, int pos_y, float spd_x, float spd_y, int floor, int ceiling, int wall_left, int wall_right, float gravity, void *userdata);
void physics_tick(physics_state *state);
void physics_jump(physics_state *state, float spd_y);
void physics_recoil(physics_state *state, float spd_x, float spd_y);
void physics_move(physics_state *state, float spd_x);
void physics_crouch(physics_state *state);
int physics_is_moving_up(physics_state *state);
int physics_is_moving_down(physics_state *state);
int physics_is_stopped(physics_state *state);
int physics_is_in_air(physics_state *state);

#endif // _PHYSICS_H