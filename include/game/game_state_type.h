#ifndef _GAME_STATE_TYPE_H
#define _GAME_STATE_TYPE_H

#include "utils/vector.h"

typedef struct scene_t scene;
typedef struct game_player_t game_player;
typedef struct ticktimer_t ticktimer;

typedef struct game_state_t {
    unsigned int run;
    unsigned int this_id, next_id;
    unsigned int tick;
    unsigned int role;
    int net_mode; // NET_MODE_NONE, NET_MODE_CLIENT, NET_MODE_SERVER
    scene *sc;
    vector objects;
    game_player *players[2];
    ticktimer *tick_timer;
} game_state;

#endif // _GAME_STATE_TYPE_H
