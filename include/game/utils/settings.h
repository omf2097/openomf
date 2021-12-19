#ifndef SETTINGS_H
#define SETTINGS_H

typedef enum fight_mode_t {
    FIGHT_MODE_NORMAL,
    FIGHT_MODE_HYPER
} fight_mode;

typedef enum knock_down_mode_t {
    KNOCK_DOWN_NONE = 0,
    KNOCK_DOWN_PUNCHES,
    KNOCK_DOWN_KICKS,
    KNOCK_DOWN_BOTH
} knock_down_mode;

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
    char *sink;
    int music_mono;
    int music_frequency;
    int music_resampler;
    int music_library;
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

typedef struct settings_tournament_t {
    char *last_name;
} settings_tournament;

typedef struct settings_advanced_t {
    int rehit_mode;
    int defensive_throws;
    int throw_range;
    int jump_height;
    int hit_pause;
    int vitality;
    int knock_down;
    int block_damage;
} settings_advanced;

typedef struct settings_keyboard_t {
    // Player one
    int ctrl_type1;
    char *joy_name1;
    int joy_offset1;
    char *key1_jump_up;
    char *key1_jump_right;
    char *key1_walk_right;
    char *key1_duck_forward;
    char *key1_duck;
    char *key1_duck_back;
    char *key1_walk_back;
    char *key1_jump_left;
    char *key1_kick;
    char *key1_punch;
    char *key1_escape;

    // Player two
    int ctrl_type2;
    char *joy_name2;
    int joy_offset2;
    char *key2_jump_up;
    char *key2_jump_right;
    char *key2_walk_right;
    char *key2_duck_forward;
    char *key2_duck;
    char *key2_duck_back;
    char *key2_walk_back;
    char *key2_jump_left;
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
    settings_advanced advanced;
    settings_tournament tournament;
    settings_keyboard keys;
    settings_network net;
} settings;

int settings_write_defaults(const char *path);
int settings_init(const char *path);
void settings_free();

void settings_load();
void settings_save();

settings *settings_get();

#endif // SETTINGS_H
