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
    int sound_on;
    int music_on;
    int stereo_on;
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
    int resindex;
    int screen_w;
    int screen_h;
    int vsync;
    int fullscreen;
    int scaling;
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

typedef struct settings_t {
    settings_video video;
    settings_sound sound;
    settings_gameplay gameplay;
} settings;

int settings_write_defaults();
int settings_init();
void settings_free();

void settings_load();
void settings_save();

settings *settings_get();

#endif // _SETTINGS_H

