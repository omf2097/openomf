#ifndef GAME_STATE_TYPE_H
#define GAME_STATE_TYPE_H

#include "engine.h"
#include "game/protos/fight_stats.h"
#include "utils/random.h"
#include "utils/vector.h"

enum
{
    RENDER_LAYER_BOTTOM = 0,
    RENDER_LAYER_MIDDLE,
    RENDER_LAYER_TOP
};

enum
{
    ROLE_SERVER,
    ROLE_CLIENT
};

enum
{
    NET_MODE_NONE,
    NET_MODE_SERVER,
    NET_MODE_CLIENT
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
    engine_init_flags *init_flags;

    // For screen shaking
    int screen_shake_horizontal;
    int screen_shake_vertical;

    // For momentary game speed switches
    int speed_slowdown_previous;
    int speed_slowdown_time;

    // Crossfade state
    int next_wait_ticks;
    int this_wait_ticks;

    // For debugging, sets fastest possible mode :)
    int warp_speed;

    int next_requires_refresh; // If next frame requires a texture refresh, this should be set to 1
    int net_mode;              // NET_MODE_NONE, NET_MODE_CLIENT, NET_MODE_SERVER
    scene *sc;
    vector objects;
    game_player *players[2];

    fight_stats fight_stats;
    void *new_state;
    struct random_t rand;
} game_state;

#endif // GAME_STATE_TYPE_H
