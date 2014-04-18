#ifndef _SETTINGS_H
#define _SETTINGS_H

typedef enum fight_mode_t {
    FIGHT_MODE_NORMAL, 
    FIGHT_MODE_HYPER
} fight_mode;

typedef enum difficulty_t {
    PUNCHING_BAG, 
    ROOKIE, 
    VETERAN, 
    WORLD_CLASS,
    CHAMPION,
    DEADLY,
    ULTIMATE
} difficulty;

typedef struct settings_sound_t {
    int stereo_reversed;
    int sound_vol;
    int music_vol;
    char *music_arena0;
    char *music_arena1;
    char *music_arena2;
    char *music_arena3;
    char *music_arena4;
    char *music_end;
    char *music_menu;
} settings_sound;

typedef struct settings_video_t {
    int screen_w;
    int screen_h;
    int vsync;
    int fullscreen;
    int scaling;
    int instant_console;
    int crossfade_on;
    char *scaler;
    int scale_factor;
} settings_video;

typedef struct settings_gameplay_t {
    int speed;
    int fight_mode;
    int power1;
    int power2;
    int hazards_on;
    int difficulty;
    int rounds;
} settings_gameplay;

typedef struct settings_keyboard_t {
    // Player one
    int ctrl_type1;
    char *key1_up;
    char *key1_down;
    char *key1_left;
    char *key1_right;
    char *key1_kick;
    char *key1_punch;
    char *key1_escape;

    // Player two
    int ctrl_type2;
    char *key2_up;
    char *key2_down;
    char *key2_left;
    char *key2_right;
    char *key2_kick;
    char *key2_punch;
    char *key2_escape;
} settings_keyboard;

typedef struct settings_network_t {
    char *net_connect_ip;
    int net_connect_port;
    int net_listen_port;
} settings_network;


typedef struct settings_t {
    settings_video video;
    settings_sound sound;
    settings_gameplay gameplay;
    settings_keyboard keys;
    settings_network net;
} settings;

int settings_write_defaults(const char *path);
int settings_init(const char *path);
void settings_free();

void settings_load();
void settings_save();

settings *settings_get();

#endif // _SETTINGS_H
