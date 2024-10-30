#ifndef SETTINGS_H
#define SETTINGS_H

typedef enum
{
    FIGHT_MODE_NORMAL,
    FIGHT_MODE_HYPER
} fight_mode;

typedef enum
{
    KNOCK_DOWN_NONE = 0,
    KNOCK_DOWN_PUNCHES,
    KNOCK_DOWN_KICKS,
    KNOCK_DOWN_BOTH
} knock_down_mode;

typedef enum
{
    PUNCHING_BAG,
    ROOKIE,
    VETERAN,
    WORLD_CLASS,
    CHAMPION,
    DEADLY,
    ULTIMATE
} difficulty;

typedef struct {
    int music_mono;
    int music_frequency;
    int music_resampler;
    int sound_vol;
    int music_vol;
} settings_sound;

typedef struct {
    int screen_w;
    int screen_h;
    int vsync;
    int fullscreen;
    int scaling;
    int instant_console;
    int crossfade_on;
} settings_video;

typedef struct {
    int speed;
    int fight_mode;
    int power1;
    int power2;
    int hazards_on;
    int difficulty;
    int rounds;
} settings_gameplay;

typedef struct {
    char *last_name;
} settings_tournament;

typedef struct {
    int rehit_mode;
    int defensive_throws;
    int throw_range;
    int jump_height;
    int hit_pause;
    int vitality;
    int knock_down;
    int block_damage;
} settings_advanced;

typedef struct {
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

typedef struct {
    char *net_connect_ip;
    char *trace_file;
    char *net_username;
    int net_connect_port;
    int net_listen_port_start;
    int net_listen_port_end;
    int net_ext_port_start;
    int net_ext_port_end;
    int net_use_upnp;
    int net_use_pmp;
} settings_network;

typedef struct {
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
void settings_free(void);

void settings_load(void);
void settings_save(void);

settings *settings_get(void);

#endif // SETTINGS_H
