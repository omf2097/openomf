#ifndef GAME_STATE_TYPE_H
#define GAME_STATE_TYPE_H

#include <stdbool.h>

#include "engine.h"
#include "formats/rec.h"
#include "game/protos/fight_stats.h"
#include "game/utils/settings.h"
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
    NET_MODE_CLIENT,
    NET_MODE_LOBBY
};

typedef struct scene_t scene;
typedef struct game_player_t game_player;
typedef struct ticktimer_t ticktimer;
typedef struct controller_t controller;

// roughly modeled after the configuration in REC files
typedef struct match_settings_t {
    uint8_t throw_range;
    uint8_t hit_pause;
    uint8_t block_damage;
    uint8_t vitality;
    uint8_t jump_height;
    knock_down_mode knock_down;
    bool rehit;
    bool defensive_throws;
    uint8_t power1;
    uint8_t power2;
    bool hazards;
    uint8_t rounds;
    bool fight_mode;
} match_settings;

typedef struct game_state_t {
    unsigned int run;
    unsigned int paused;
    unsigned int this_id;
    unsigned int next_id;
    unsigned int next_next_id;
    uint32_t tick;
    uint32_t int_tick; // never adjusted, used in ping calculation
    unsigned int role;
    unsigned int speed;
    engine_init_flags *init_flags;

    match_settings match_settings;

    // For screen shaking
    int screen_shake_horizontal;
    int screen_shake_vertical;

    // For momentary game speed switches
    int speed_slowdown_previous;
    int speed_slowdown_time;

    int hit_pause;

    // Crossfade state
    int next_wait_ticks;
    int this_wait_ticks;

    // Newsroom HAR screencaps
    bool hide_ui;

    // For debugging, sets fastest possible mode :)
    int warp_speed;

    int net_mode; // NET_MODE_NONE, NET_MODE_CLIENT, NET_MODE_SERVER
    scene *sc;
    vector objects;
    vector sounds;
    game_player *players[2];

    fight_stats fight_stats;
    void *new_state;
    bool clone;
    int delay;
    struct random_t rand;

    sd_rec_file *rec;

    controller *menu_ctrl;
} game_state;

#endif // GAME_STATE_TYPE_H
