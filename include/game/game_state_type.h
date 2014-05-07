#ifndef _GAME_STATE_TYPE_H
#define _GAME_STATE_TYPE_H

#include "utils/vector.h"

enum {
    RENDER_LAYER_BOTTOM = 0,
    RENDER_LAYER_MIDDLE,
    RENDER_LAYER_TOP
};

enum {
    ROLE_CLIENT,
    ROLE_SERVER
};

enum {
    NET_MODE_NONE,
    NET_MODE_CLIENT,
    NET_MODE_SERVER
};

typedef struct scene_t scene;
typedef struct game_player_t game_player;
typedef struct ticktimer_t ticktimer;

typedef struct game_state_t {
    unsigned int run;
    unsigned int paused;
    unsigned int this_id;
    unsigned int next_id;
    unsigned int next_next_id;
    unsigned int tick;
    unsigned int int_tick; // never adjusted, used in ping calculation
    unsigned int role;
    unsigned int speed;

    // For screen shaking
    int screen_shake_horizontal;
    int screen_shake_vertical;

    // Crossfade state
    int next_wait_ticks;
    int this_wait_ticks;

    int next_requires_refresh; // If next frame requires a texture refresh, this should be set to 1
    int net_mode; // NET_MODE_NONE, NET_MODE_CLIENT, NET_MODE_SERVER
    scene *sc;
    vector objects;
    game_player *players[2];
    ticktimer *tick_timer;
} game_state;

#endif // _GAME_STATE_TYPE_H
